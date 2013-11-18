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

#include "passphrase.h"


/* Number of seconds to sleep on error, so the user has time to read the error message */
#ifndef ERROR_SLEEP
#define ERROR_SLEEP  2
#endif


/**
 * Quit function for timeout
 * 
 * @param  signal  The signal the program received
 */
void timeout_quit(int signal);

/**
 * Quit function for user aborts
 * 
 * @param  signal  The signal the program received
 */
void user_quit(int signal);
