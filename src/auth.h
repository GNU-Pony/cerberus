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
#ifndef __AUTH_H__
#define __AUTH_H__



#if AUTH == 0

#define  close_login_session(...)  /* do nothing */
#define  initialise_login(...)     (void) hostname
#define  authenticate_login(...)   1
#define  verify_account(...)       /* do nothing */
#define  open_login_session(...)   /* do nothing */

#elif AUTH == 1

#include "auth/pam.h"
#define  close_login_session  close_session_pam
#define  initialise_login     initialise_pam
#define  authenticate_login   authenticate_pam
#define  verify_account       verify_account_pam
#define  open_login_session   open_session_pam

#endif


#endif

