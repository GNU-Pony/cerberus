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


/* The number of seconds before the program times out */
#ifndef TIMEOUT_SECONDS
#define TIMEOUT_SECONDS  60
#endif



/**
 * Mane method
 * 
 * @param   argc  The number of command line arguments
 * @param   argv  The command line arguments
 * @return        Return code
 */
int main(int argc, char** argv)
{
  char* username = NULL;
  char* hostname = NULL;
  char* passphrase = NULL;
  char preserve_env = 0;
  char skip_auth = 0;
  
  /* Disable echoing */
  disable_echo();
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
	    if ((c == 'V') || (c == 'H'))
	      ;
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
	      fprintf(stderr, "%s: unrecognised options: -%c\n", *argv, c);
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
  if (username == 0)
    {
      printf("%s: no username specified\n", *argv);
      reenable_echo();
      return 2;
    }
  
  
  /* Print ant we want a passphrase, if -f has not been used */
  if (skip_auth == 0)
    {
      printf("Passphrase: ");
      fflush(stdout);
    }
  /* Done early to make to program look like it is even faster than it is */
  
  
  /* Make sure nopony is spying */
  secure_tty();
  
  
  /* Set up clean quiting and time out */
  signal(SIGALRM, timeout_quit);
  signal(SIGQUIT, user_quit);
  signal(SIGINT, user_quit);
  siginterrupt(SIGALRM, 1);
  siginterrupt(SIGQUIT, 1);
  siginterrupt(SIGINT, 1);
  alarm(TIMEOUT_SECONDS);
  
  
  /* Get the passphrase, if -f has not been used */
  if (skip_auth == 0)
    {
      passphrase = get_passphrase();
      printf("\n");
    }
  
  /* Passphrase entered, turn off timeout */
  alarm(0);
  
  /* Wipe and free the passphrase from the memory */
  if (skip_auth == 0)
    {
      long i;
      for (i = 0; *(passphrase + i); i++)
	*(passphrase + i) = 0;
      free(passphrase);
    }
  
  
  /* Reset terminal settings */
  reenable_echo();
  
  return 0;
}

