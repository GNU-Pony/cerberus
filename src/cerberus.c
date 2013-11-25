/**
 * cerberus – Minimal login program
 * 
 * Copyright © 2013  Mattias Andrée (maandree@member.fsf.org)
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

/* TODO use log */


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
 * The passphrase
 */
char* passphrase = NULL;


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
      if (fd)
	dup2(fd, 0);
      dup2(fd, 1);
      dup2(fd, 2);
    }
  
  /* Reset terminal ownership and mode */
  chown_tty(0, tty_group, 0);
  
  /* Close login session */
  close_login_session();
  
  return 0;
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
  char skip_auth = 0;
  int ret;
  #ifdef USE_TTY_GROUP
  struct group* group;
  #endif
  
  
  /* Disable echoing */
  passphrase_disable_echo();
  /* This should be done as early and quickly as possible so as little
     as possible of the passphrase gets leaked to the output if the user
     begins entering the passphrase directly after the username. */
  
  
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
  
  
  /* Only root may bypass authentication */
  if (skip_auth && getuid())
    {
      printf("%s: only root by use the -f option\n", *argv);
      sleep(ERROR_SLEEP);
      _exit(2);
    }
  
  
  /* Print ant we want a passphrase, if -f has not been used */
  if (skip_auth == 0)
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
  
  /* Redisable echoing */
  passphrase_disable_echo();
  
  
  /* Set up clean quiting and time out */
  signal(SIGALRM, timeout_quit);
  signal(SIGQUIT, user_quit);
  signal(SIGINT, user_quit);
  siginterrupt(SIGALRM, 1);
  siginterrupt(SIGQUIT, 1);
  siginterrupt(SIGINT, 1);
  alarm(TIMEOUT_SECONDS);
  
  
  /* Get user information */
  if ((entry = getpwnam(username)) == NULL)
    {
      if (errno)
	perror("getpwnam");
      else
	printf("User does not exist\n");
      sleep(ERROR_SLEEP);
      _exit(1);
    }
  endpwent();
  username = entry->pw_name;
  
  
  /* Verify passphrase or other token, if -f has not been used */
  initialise_login(hostname, username, read_passphrase);
  if ((skip_auth == 0) && authenticate_login())
    printf("(auto-authenticated)\n");
  
  /* Passphrase entered, turn off timeout */
  alarm(0);
  
  /* Wipe and free the passphrase from the memory */
  if (passphrase)
    {
      long i;
      for (i = 0; *(passphrase + i); i++)
	*(passphrase + i) = 0;
      free(passphrase);
    }
  
  /* Reset terminal settings */
  passphrase_reenable_echo();
  
  
  /* Verify account, such as that it is enabled */
  verify_account();
  
  
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


/**
 * Read passphrase from the terminal
 * 
 * @return  The entered passphrase
 */
char* read_passphrase(void)
{
  passphrase = passphrase_read();
  if (passphrase == NULL)
    {
      perror("passphrase_read");
      sleep(ERROR_SLEEP);
      _exit(1);
    }
  return passphrase;
}

