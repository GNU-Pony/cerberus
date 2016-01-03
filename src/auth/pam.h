/**
 * cerberus – Minimal login program
 * 
 * Copyright © 2013, 2014, 2015, 2016  Mattias Andrée (maandree@member.fsf.org)
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
#ifndef CERBERUS_AUTH_PAM_H
#define CERBERUS_AUTH_PAM_H


/**
 * Initialise PAM
 * 
 * @param  remote    The remote computer, {@code NULL} for local login
 * @param  username  The username of the user to log in to
 * @param  reader    Function that can be used to read a passphrase from the terminal
 */
void initialise_pam(char* remote, char* username, char* (*reader)(void));

/**
 * Verify that the account may be used
 */
void verify_account_pam(void);

/**
 * Open PAM session
 */
void open_session_pam(void);

/**
 * Close PAM session
 */
void close_session_pam(void);

/**
 * Perform token authentication
 * 
 * @return  0: failed, 1: success, 2: auto-authenticated
 */
char authenticate_pam(void);


#endif

