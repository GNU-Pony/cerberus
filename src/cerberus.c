/**
 * cerberus – Minimal login program
 * 
 * Copyright © 2013, 2014, 2015, 2016, 2020  Mattias Andrée (maandree@kth.se)
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
#include "cerberus.h"

#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>


/**
 * Index of the hook used when the user logs in.
 */
#define HOOK_LOGIN  0

/**
 * Index of the hook used when the user logs out.
 */
#define HOOK_LOGOUT  1

/**
 * Index of the hook used when the user was denied access.
 */
#define HOOK_DENIED  2

/**
 * Index of the hook used to verify that the user may log in.
 */
#define HOOK_VERIFY  3



#ifdef USE_TTY_GROUP
/**
 * The group ID for the `tty` group
 */
static gid_t tty_group = 0;
#endif

/**
 * The user's entry in the password file
 */
static struct passwd* entry;

/**
 * The process ID of the child process, 0 if none
 */
pid_t child_pid = 0;

/**
 * Whether authentication should be skipped
 */
static char skip_auth = 0;

#if AUTH > 0
/**
 * The passphrase
 */
char* passphrase = NULL;
#endif


/**
 * Sleep without letting the user stop it
 * 
 * @param  s  The number of seconds to sleep
 */
static void xsleep(unsigned int s)
{
  sigset_t sigset;
  sigset_t old_sigset;
  
  sigfillset(&sigset);
  sigprocmask(SIG_BLOCK, &sigset, &old_sigset);
  
  while ((s = sleep(s)));
  
  sigprocmask(SIG_SETMASK, &old_sigset, NULL);
}


/**
 * Mane method
 * 
 * @param   argc  The number of command line arguments
 * @param   argv  The command line arguments
 * @return        Return code
 */
int main(int argc, char** argv)
{
  char* tty_device = ttyname(STDIN_FILENO);
  
  do_login(argc, argv);
  
  /* Ignore signals */
  signal(SIGQUIT, SIG_IGN);
  signal(SIGINT, SIG_IGN);
  
  /* Wait for the login shell and all grandchildren to exit */
  while ((wait(NULL) == -1) && (errno == EINTR))
    ;
  
  /* Regain access to the terminal */
  if (tty_device)
    {
      int fd = open(tty_device, O_RDWR | O_NONBLOCK);
      if (fd != 0)  dup2(fd, 0);
      if (fd != 1)  dup2(fd, 1);
      if (fd != 2)  dup2(fd, 2);
    }
  
  /* Reset terminal ownership and mode */
  chown_tty(0, tty_group, 0);
  
  /* Close login session */
  close_login_session();
  
  /* Run logout hook */
  exec_hook(HOOK_LOGOUT, argc, argv);
  return 0;
}


/**
 * Exec /etc/cerberusrc
 * 
 * @param  hook  The ID of the hook to run
 * @param  argc  The number of command line arguments
 * @param  argv  The command line arguments
 */
void exec_hook(int hook, int argc, char** argv)
{
  static char cerberusrc[] = CERBERUSRC;
  static char hooks[][7] =
    {
      [HOOK_LOGIN]  = "login",
      [HOOK_LOGOUT] = "logout",
      [HOOK_DENIED] = "denied",
      [HOOK_VERIFY] = "verify",
    };
  char** args;
  int i;
  
  args = malloc((size_t)(argc + 2) * sizeof(char*));
  if (args == NULL)
    {
      perror("malloc");
      return;
    }
  
  args[0] = cerberusrc;
  args[1] = hooks[hook];
  for (i = 1; i < argc; i++)
    args[i + 1] = argv[i];
  args[argc + 1] = NULL;
  
  execv(CERBERUSRC, args);
}


/**
 * Fork-exec-wait /etc/cerberusrc
 * 
 * @param   hook  The ID of the hook to run
 * @param   argc  The number of command line arguments
 * @param   argv  The command line arguments
 * @return        The exit value of the hook
 */
int fork_exec_wait_hook(int hook, int argc, char** argv)
{
  pid_t pid, reaped;
  int status;
  pid = fork();
  if (pid == -1)
    return -1;
  if (pid == 0)
    {
      close(STDIN_FILENO);
      exec_hook(hook, argc, argv);
      _exit(138);
    }
  for (;;)
    {
      reaped = wait(&status);
      if (reaped == -1)
	{
	  perror("wait");
	  return -1;
	}
      if (reaped == pid)
	return status == 138 ? -1 : status;
    }
}


/**
 * Do everything before the fork and do everything the exec fork
 * 
 * @param   argc  The number of command line arguments
 * @param   argv  The command line arguments
 */
void do_login(int argc, char** argv)
{
  char* username = NULL;
  char* hostname = NULL;
  char preserve_env = 0;
  int ret;
  #ifdef USE_TTY_GROUP
  struct group* group;
  #endif
  
  
  #if AUTH > 0
  /* Disable echoing */
  passphrase_disable_echo1(STDIN_FILENO /* Will be the terminal. */);
  /* This should be done as early and quickly as possible so as little
     as possible of the passphrase gets leaked to the output if the user
     begins entering the passphrase directly after the username. */
  #endif
  
  
  /* Set process group ID */
  setpgrp();
  
  
  /* Parse command line arguments */
  {
    char double_dashed = 0;
    char hostname_on_next = 0;
    int i;
    for (i = 1; i < argc; i++)
      {
	char *arg = *(argv + i);
	char c;
	
	if (*arg == 0)
	  ;
	else if ((*arg == '-') && (double_dashed == 0))
	  while ((c = *(++arg)))
	    if (c == 'H')
	      ;
	    else if (c == 'V')
	      _exit(2);
	    else if (c == 'p')
	      preserve_env = 1;
	    else if (c == 'h')
	      {
		if (*(arg + 1))
		  hostname = arg + 1;
		else
		  hostname_on_next = 1;
		break;
	      }
	    else if (c == 'f')
	      {
		if (*(arg + 1))
		  username = arg + 1;
		skip_auth = 1;
		break;
	      }
	    else if (c == '-')
	      {
		double_dashed = 1;
		break;
	      }
	    else
	      printf("%s: unrecognised options: -%c\n", *argv, c);
	else if (hostname_on_next)
	  {
	    hostname = arg;
	    hostname_on_next = 0;
	  }
	else
	  username = arg;
      }
  }
  
  
  /* Change that a username has been specified */
  if (username == NULL)
    {
      printf("%s: no username specified\n", *argv);
      sleep(ERROR_SLEEP);
      _exit(2);
    }
  
  
  /* Verify that the user may login */
  ret = fork_exec_wait_hook(HOOK_VERIFY, argc, argv);
  if ((ret >= 0) && WIFEXITED(ret) && (WEXITSTATUS(ret) == 1))
    {
      sleep(ERROR_SLEEP);
      _exit(2);
    }
  
  
  if (skip_auth)
    {
      /* Reset terminal settings */
      passphrase_reenable_echo1(STDIN_FILENO);
      
      /* Only root may bypass authentication */
      if (getuid())
	{
	  printf("%s: only root by use the -f option\n", *argv);
	  sleep(ERROR_SLEEP);
	  _exit(2);
	}
    }
  /* Print ant we want a passphrase, if -f has not been used */
  else
    {
      printf("Passphrase: ");
      fflush(stdout);
    }
  /* Done early to make to program look like it is even faster than it is */
  
  
  /* Make sure nopony is spying */
  #ifdef USE_TTY_GROUP
  if ((group = getgrnam(TTY_GROUP)))
    tty_group = group->gr_gid;
  endgrent();
  #endif
  secure_tty(tty_group);
  
  #if AUTH > 0
  if (skip_auth == 0)
    {
      /* Redisable echoing */
      passphrase_disable_echo1(STDIN_FILENO);
    }
  #endif
  
  
  /* Set up clean quiting and time out */
  signal(SIGALRM, timeout_quit);
  signal(SIGQUIT, user_quit);
  signal(SIGINT, user_quit);
  siginterrupt(SIGALRM, 1);
  siginterrupt(SIGQUIT, 1);
  siginterrupt(SIGINT, 1);
  #if AUTH > 0
  alarm(TIMEOUT_SECONDS);
  #endif
  
  
  /* Get user information */
  if ((entry = getpwnam(username)) == NULL)
    {
      if (errno == EIO /* seriously...? */ || !errno)
	printf("User does not exist\n");
      else if (errno)
	perror("getpwnam");
      sleep(ERROR_SLEEP);
      _exit(1);
    }
  endpwent();
  username = entry->pw_name;
  
  
  /* Verify passphrase or other token, if -f has not been used */
  ret = 2;
  #if AUTH == 0
  (void) hostname;
  #else
  initialise_login(hostname, username, read_passphrase);
  if (skip_auth == 0)
    ret = authenticate_login();
  /* Passphrase entered, turn off timeout */
  alarm(0);
  #endif
  if (ret == 2)
    printf("(auto-authenticated)\n");
  if (ret == 0)
    {
      preexit();
      fork_exec_wait_hook(HOOK_DENIED, argc, argv);
      xsleep(FAILURE_SLEEP);
      _exit(1);
    }
  
  preexit();
  
  
  /* Verify account, such as that it is enabled */
  verify_account();
  
  
  /* Run login hook */
  fork_exec_wait_hook(HOOK_LOGIN, argc, argv);
  
  
  /* Partial login */
  chown_tty(entry->pw_uid, tty_group, 0);
  chdir_home(entry);
  ensure_shell(entry);
  set_environ(entry, preserve_env);
  open_login_session();
  
  
  /* Stop signal handling */
  signal(SIGALRM, SIG_DFL);
  signal(SIGQUIT, SIG_DFL);
  signal(SIGTSTP, SIG_IGN);
  
  
  child_pid = fork();
  /* vfork cannot be used as the child changes the user,
     the parent would not be able to chown the TTY */
  
  if (child_pid == -1)
    {
      perror("fork");
      close_login_session();
      sleep(ERROR_SLEEP);
      _exit(1);
    }
  else if (child_pid)
    return; /* Do not go beyond this in the parent */
  
  /* In case the shell does not do this */
  setsid();
  
  /* Set controlling terminal */
  if (ioctl(STDIN_FILENO, TIOCSCTTY, 1))
    perror("TIOCSCTTY");
  signal(SIGINT, SIG_DFL);
  
  /* Partial login */
  ret = entry->pw_uid
    ? initgroups(username, entry->pw_gid) /* supplemental groups for user, can require network     */
    : setgroups(0, NULL);                 /* supplemental groups for root, does not require netork */
  if (ret == -1)
    {
      perror(entry->pw_uid ? "initgroups" : "setgroups");
      sleep(ERROR_SLEEP);
      _exit(1);
    }
  set_user(entry);
  exec_shell(entry);
}



#if AUTH > 0
/**
 * Called before the process exits, to do cleanup
 */
void preexit(void)
{
  if (skip_auth == 0)
    {
      /* Wipe and free the passphrase from the memory */
      destroy_passphrase();
      
      /* Reset terminal settings */
      passphrase_reenable_echo1(STDIN_FILENO);
    }
}


/**
 * Read passphrase from the terminal
 * 
 * @return  The entered passphrase
 */
char* read_passphrase(void)
{
  passphrase = passphrase_read2(STDIN_FILENO, PASSPHRASE_READ_EXISTING);
  if (passphrase == NULL)
    {
      perror("passphrase_read");
      sleep(ERROR_SLEEP);
      _exit(1);
    }
  return passphrase;
}
#endif


#ifdef __GNUC__
# pragma GCC optimize "-O0"
#endif


/**
 * Wipe and free the passphrase if it is allocated
 */
void destroy_passphrase(void)
{
  if (passphrase)
    {
      passphrase_wipe(passphrase, strlen(passphrase));
      free(passphrase);
      passphrase = NULL;
    }
}


/**
 * Wipe the passphrase when the program exits
 */
#ifdef __GNUC__
__attribute__((destructor))
#endif
static void passphrase_destructor(void)
{
  destroy_passphrase();
}

