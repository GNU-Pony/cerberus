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
#ifndef CERBERUS_SECURITY_H
#define CERBERUS_SECURITY_H

#include <sys/types.h>


/**
 * Secure the TTY from spying
 * 
 * @param  group  The group, -1 for unchanged
 */
void secure_tty(gid_t group);

/**
 * Set ownership and mode of the TTY
 * 
 * @param  owner      The owner, -1 for unchanged
 * @param  group      The group, -1 for unchanged
 * @param  with_fail  Abort on failure
 */
void chown_tty(uid_t owner, gid_t group, char with_fail);


#endif

