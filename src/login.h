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
#ifndef __LOGIN_H__
#define __LOGIN_H__

#include <pwd.h>


/**
 * Change directory to the user's home directory
 * 
 * @param  entry  The user entry in the password file
 */
void chdir_home(struct passwd* entry);

/**
 * Make sure the shell to use is definied
 * 
 * @param  entry  The user entry in the password file
 */
void ensure_shell(struct passwd* entry);

/**
 * Set environment variables
 * 
 * @param  entry         The user entry in the password file
 * @param  preserve_env  Whether to preserve the environment
 */
void set_environ(struct passwd* entry, char preserve_env);


#endif

