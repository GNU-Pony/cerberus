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
#ifndef NO_SHADOW
# ifndef HAVE_SHADOW
#  define HAVE_SHADOW
# endif
#endif

#define _XOPEN_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <termios.h>
#ifdef HAVE_SHADOW
#include <shadow.h>
#endif

#include "../config.h"

#include "crypt.h"


#if !defined(__USE_SVID) && !defined(__USE_MISC) && !defined(__USE_XOPEN_EXTENDED) 
#define endpwent()  /* do nothing */
#endif


/**
 * Function that can be used to read a passphrase from the terminal
 */
static char* (*passphrase_reader)(void) = NULL;

/**
 * The username of the user to log in to
 */
static char* login_username;



/**
 * Initialise crypt authentication module
 * 
 * @param  remote    The remote computer, {@code NULL} for local login
 * @param  username  The username of the user to log in to
 * @param  reader    Function that can be used to read a passphrase from the terminal
 */
void initialise_crypt(char* remote, char* username, char* (*reader)(void))
{
  (void) remote;
  
  login_username = username;
  passphrase_reader = reader;
}


/**
 * Perform token authentication
 * 
 * @return  0: failed, 1: success, 2: auto-authenticated
 */
char authenticate_crypt(void)
{
#ifdef HAVE_SHADOW
  struct spwd* shadow_entry = NULL;
#endif
  struct passwd* passwd_entry = NULL;
  char* crypted;
  char* entered;
  struct termios stty;
  
#ifdef HAVE_SHADOW
  shadow_entry = getspnam(login_username);
  endspent();
  
  if (shadow_entry)
    crypted = shadow_entry->sp_pwdp;
  else
    {
#endif
      passwd_entry = getpwnam(login_username);
      if (passwd_entry)
	crypted = passwd_entry->pw_passwd;
      else
	{
	  perror("getpwnam");
	  endpwent();
	  sleep(ERROR_SLEEP);
	  _exit(1);
	}
      endpwent();
#ifdef HAVE_SHADOW
    }
#endif
  
  if (!(crypted && *crypted)) /* empty means that no passphrase is required (not even Enter) */
    return 2;
  
  entered = crypt(passphrase_reader(), crypted /* salt argument stops parsing when encrypted begins */);
  if (entered && !strcmp(entered, crypted))
    return 1;
  
  /* Clear ISIG (and everything else) to prevent the user
   * from skipping the brute force protection sleep. */
  tcgetattr(STDIN_FILENO, &stty);
  stty.c_lflag = 0;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &stty);
  
  printf("Incorrect passphrase\n");
  return 0;
}

