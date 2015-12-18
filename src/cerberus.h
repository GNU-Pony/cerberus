/**
 * cerberus – Minimal login program
 * 
 * Copyright © 2013, 2014, 2015  Mattias Andrée (maandree@member.fsf.org)
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
#ifndef CERBERUS_CERBERUS_H
#define CERBERUS_CERBERUS_H


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pwd.h>
#include <errno.h>
#include <stropts.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <grp.h>

#if AUTH > 0
#include <passphrase.h>
#endif

#include "config.h"

#include "quit.h"
#include "login.h"
#include "security.h"
#include "auth.h"


#ifndef USE_TTY_GROUP
#define tty_group  0
#endif


int fork_exec_wait_hook(int hook, int argc, char** argv);
void exec_hook(int hook, int argc, char** argv);
void do_login(int argc, char** argv);

#if AUTH > 0
void preexit(void);
char* read_passphrase(void);
#else
#define preexit()          /* do nothing */
#define read_passphrase()  NULL
#endif

void destroy_passphrase(void) __attribute__((optimize("-O0")));


#endif

