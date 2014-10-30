/**
 * cerberus – Minimal login program
 * 
 * Copyright © 2013, 2014  Mattias Andrée (maandree@member.fsf.org)
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <security/pam_appl.h>
#include <security/pam_misc.h>

#include "../config.h"

#include "pam.h"


#define __failed(RC)  ((RC) != PAM_SUCCESS)


static void quit_pam(int sig) __attribute__((noreturn));

static int conv_pam(int num_msg, const struct pam_message** msg, struct pam_response** resp, void* appdata_ptr);


/**
 * Old signal action for SIGHUP
 */
struct sigaction signal_action_hup;

/**
 * Old signal action for SIGTERM
 */
struct sigaction signal_action_term;

/**
 * The process ID of the child process, 0 if none
 */
extern pid_t child_pid;

/**
 * The PAM handle
 */
static pam_handle_t* handle = NULL;

/**
 * The PAM convention
 */
static struct pam_conv conv = { conv_pam, NULL };

/**
 * Whether the user was auto-authenticated
 */
static char auto_authenticated = 1;

/**
 * Function that can be used to read a passphrase from the terminal
 */
static char* (*passphrase_reader)(void) = NULL;


# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wsuggest-attribute=pure"
/**
 * Exit if a PAM instruction failed
 * 
 * @param  rc  What the PAM instruction return
 */
static void do_pam(int rc)
{
  if (__failed(rc))
    {
      const char* msg = pam_strerror(handle, rc);
      if (msg)
	fprintf(stderr, "%s\n", msg);
      pam_end(handle, rc);
      sleep(ERROR_SLEEP);
      _exit(1);
    }
}
# pragma GCC diagnostic pop


/**
 * Initialise PAM
 * 
 * @param  remote    The remote computer, {@code NULL} for local login
 * @param  username  The username of the user to log in to
 * @param  reader    Function that can be used to read a passphrase from the terminal
 */
void initialise_pam(char* remote, char* username, char* (*reader)(void))
{
  passphrase_reader = reader;
  
  if (pam_start(remote ? "remote" : "local", username, &conv, &handle) != PAM_SUCCESS)
    {
      fprintf(stderr, "Cannot initialise PAM\n");
      sleep(ERROR_SLEEP);
      _exit(1);
    }
  
  do_pam(pam_set_item(handle, PAM_RHOST, remote ?: "localhost"));
  do_pam(pam_set_item(handle, PAM_TTY, ttyname(STDIN_FILENO) ?: "(none)"));
  do_pam(pam_set_item(handle, PAM_USER_PROMPT, "Username: "));
}


/**
 * Verify that the account may be used
 */
void verify_account_pam(void)
{
  /* FIXME pam_acct_mgmt exits the program, but freezes if PAM_USER_PROMPT has not been set. */
  /*       however, if -f is used there is no problem. */
  /*
  int rc = pam_acct_mgmt(handle, 0);
  if (rc == PAM_NEW_AUTHTOK_REQD)
    rc = pam_chauthtok(handle, PAM_CHANGE_EXPIRED_AUTHTOK);
  do_pam(rc);
  */
}


/**
 * Open PAM session
 */
void open_session_pam(void)
{
  int rc;
  char** env;
  struct sigaction signal_action;
  
  do_pam(pam_setcred(handle, PAM_ESTABLISH_CRED));
  
  if (__failed(rc = pam_open_session(handle, 0)))
    {
      pam_setcred(handle, PAM_DELETE_CRED);
      do_pam(rc);
    }
  
  if (__failed(rc = pam_setcred(handle, PAM_REINITIALIZE_CRED)))
    {
      pam_close_session(handle, 0);
      do_pam(rc);
    }
  
  memset(&signal_action, 0, sizeof(signal_action));
  signal_action.sa_handler = SIG_IGN;
  sigaction(SIGINT, &signal_action, NULL);
  sigaction(SIGHUP, &signal_action, &signal_action_hup);
  signal_action.sa_handler = quit_pam;
  sigaction(SIGHUP, &signal_action, NULL);
  sigaction(SIGTERM, &signal_action, &signal_action_term);
  
  for (env = pam_getenvlist(handle); env && *env; env++)
    if (putenv(*env))
      {
	pam_setcred(handle, PAM_DELETE_CRED);
	pam_end(handle, pam_close_session(handle, 0));
	sleep(ERROR_SLEEP);
	_exit(1);
      }
}


/**
 * Close PAM session
 */
void close_session_pam(void)
{
  sigaction(SIGHUP, &signal_action_hup, NULL);
  sigaction(SIGTERM, &signal_action_term, NULL);
  
  pam_setcred(handle, PAM_DELETE_CRED);
  pam_end(handle, pam_close_session(handle, 0));
}


/**
 * Signal handler for cleanly exit PAM session
 * 
 * @param  sig  The received signal
 */
void quit_pam(int sig)
{
  if (child_pid)
    kill(-child_pid, sig);
  if (sig == SIGTERM)
    kill(-child_pid, SIGHUP);
  
  pam_setcred(handle, PAM_DELETE_CRED);
  pam_end(handle, pam_close_session(handle, 0));
  
  _exit(sig);
}


/**
 * Perform token authentication
 * 
 * @return  0: failed, 1: success, 2: auto-authenticated
 */
char authenticate_pam(void)
{
  int rc;
  
  if (__failed(rc = pam_authenticate(handle, 0)))
    {
      /* Clear ISIG (and everything else) to prevent the user
       * from skipping the brute force protection sleep. */
      struct termios stty;
      tcgetattr(STDIN_FILENO, &stty);
      stty.c_lflag = 0;
      tcsetattr(STDIN_FILENO, TCSAFLUSH, &stty);
      
      printf("Incorrect passphrase\n");
      pam_end(handle, rc);
      return 0;
    }
  
  return auto_authenticated ? 2 : 1;
}


/**
 * Callback function for converation between PAM this application
 * 
 * @param   num_msg      Number of pointers in the array `msg`
 * @param   msg          Message from PAM
 * @param   resp         Pointer to responses to PAM for by index corresponding messages
 * @param   appdata_ptr  (Not used)
 * @return               `PAM_SUCCESS`, `PAM_CONV_ERR` or `PAM_BUF_ERR`
 */
int conv_pam(int num_msg, const struct pam_message** msg, struct pam_response** resp, void* appdata_ptr)
{
  int i;
  
  (void) appdata_ptr;
  
  *resp = calloc((size_t)num_msg, sizeof(struct pam_response));
  
  for (i = 0; i < num_msg; i++)
    {
      ((*resp) + i)->resp = NULL;
      ((*resp) + i)->resp_retcode = 0;
      
      if ((**(msg + i)).msg_style == PAM_PROMPT_ECHO_OFF)
	{
	  (*resp + i)->resp = passphrase_reader();
	  auto_authenticated = 0;
	}
    }
  
  return PAM_SUCCESS;
}

