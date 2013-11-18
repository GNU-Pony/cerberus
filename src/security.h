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
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>


#ifndef TTY_PERM
#define TTY_PERM  0600
#endif

#ifndef FAILURE_SLEEP
#define FAILURE_SLEEP  5
#endif


/**
 * Secure the TTY from spying
 */
void secure_tty(void);

/**
 * Set ownership and mode of the TTY
 * 
 * @param  owner      The owner
 * @param  group      The group
 * @param  with_fail  Abort on failure
 */
void chown_tty(int owner, int group, int with_fail);

