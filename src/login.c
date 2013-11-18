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
#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "config.h"

#include "login.h"


/**
 * The environment variables
 */
extern char** environ;


/**
 * Change directory to the user's home directory
 * 
 * @param  entry  The user entry in the password file
 */
void chdir_home(struct passwd* entry)
{
  if (chdir(entry->pw_dir))
    {
      perror("chdir");
      if (chdir(DEFAULT_HOME))
	{
	  perror("chdir");
	  sleep(ERROR_SLEEP);
	  _exit(1);
	}
      entry->pw_dir = DEFAULT_HOME;
    }
}


/**
 * Make sure the shell to use is definied
 * 
 * @param  entry  The user entry in the password file
 */
void ensure_shell(struct passwd* entry)
{
  if ((entry->pw_shell && *(entry->pw_shell)) == 0)
    entry->pw_shell = DEFAULT_SHELL;
}


/**
 * Set environment variables
 * 
 * @param  entry         The user entry in the password file
 * @param  preserve_env  Whether to preserve the environment
 */
void set_environ(struct passwd* entry, char preserve_env)
{
  char* _term = getenv("TERM");
  char* term = NULL;
  if (_term)
    {
      int n = 0, i;
      while (*(_term + n++))
	;
      term = malloc(n * sizeof(char));
      if (term == NULL)
	{
	  perror("malloc");
	  sleep(ERROR_SLEEP);
	  _exit(1);
	}
      for (i = 0; i < n; i++)
	*(term + i) = *(_term + i);
    }
  
  if (preserve_env == 0)
    {
      environ = malloc(sizeof(char*));
      if (environ == NULL)
	{
	  perror("malloc");
	  sleep(ERROR_SLEEP);
	  _exit(1);
	}
      *environ = NULL;
    }
  
  setenv("HOME", entry->pw_dir, 1);
  setenv("USER", entry->pw_name, 1);
  setenv("LOGUSER", entry->pw_name, 1);
  setenv("SHELL", entry->pw_shell, 1);
  setenv("TERM", term ?: DEFAULT_TERM, 1);
  setenv("PATH", entry->pw_uid ? PATH : PATH_ROOT, 1);
  
  if (term)
    free(term);
}

