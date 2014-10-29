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
#ifndef CERBERUS_LOGIN_H
#define CERBERUS_LOGIN_H

#include <pwd.h>


/**
 * Set the user gid and uid
 * 
 * @param  entry  The user entry in the password file
 */
void set_user(struct passwd* entry);

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

/**
 * Replace the current image with the user's login shell
 * 
 * @param  entry  The user entry in the password file
 */
void exec_shell(struct passwd* entry) __attribute__((noreturn));


#endif

