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
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>


/**
 * Reads the passphrase from stdin
 * 
 * @return  The passphrase, should be `free`:ed
 */
char* get_passphrase(void);

/**
 * Disable echoing and do anything else to the terminal settnings `get_passphrase` requires
 */
void disable_echo();

/**
 * Undo the actions of `disable_echo`
 */
void reenable_echo();

