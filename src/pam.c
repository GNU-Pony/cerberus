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
#include <stdio.h>
#include <unistd.h>
#include <security/pam_appl.h>
#include <security/pam_misc.h>

#include "config.h"

#include "pam.h"


/**
 * The PAM handle
 */
static pam_handle_t* handle = NULL;

/**
 * The PAM convention
 */
static struct pam_conv conv = { misc_conv, NULL };


/**
 * Exit if a PAM instruction failed
 * 
 * @param  rc  What the PAM instruction return
 */
static void do_pam(int rc)
{
  if (rc != PAM_SUCCESS)
    {
      const char* msg = pam_strerror(handle, rc);
      if (msg)
	fprintf(stderr, "%s\n", msg);
      pam_end(handle, rc);
      sleep(ERROR_SLEEP);
      _exit(1);
    }
}


/**
 * Initialise PAM
 * 
 * @param  remote    The remote computer, {@code NULL} for local login
 * @param  username  The username of the user to log in to
 */
void initialise_pam(char* remote, char* username)
{
  if (pam_start(remote ? "remote" : "local", username, &conv, &handle) != PAM_SUCCESS)
    {
      fprintf(stderr, "Cannot initialise PAM\n");
      sleep(ERROR_SLEEP);
      _exit(1);
    }
  
  do_pam(pam_set_item(handle, PAM_RHOST, remote ?: "localhost"));
  do_pam(pam_set_item(handle, PAM_TTY, ttyname(STDIN_FILENO) ?: "(none)"));
}

