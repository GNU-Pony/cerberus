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
#include "security.h"


#ifndef TTY_PERM
#define TTY_PERM  0600
#endif

#ifndef FAILURE_SLEEP
#define FAILURE_SLEEP  5
#endif


#define fail(FUNC)  ({ perror(#FUNC); sleep(FAILURE_SLEEP); _exit(1); })



/**
 * Secure the TTY from spying
 */
void secure_tty(void) /* TODO /dev/vcs[a][0-9]+ */
{
  struct termios tty;
  struct termios saved_tty;
  char* tty_device;
  int fd, i;
  
  /* Take owner ship of this TTY */
  if (fchown(STDIN_FILENO, 0, 0))
    fail(fchown);
  
  /* Restrict others from using this TTY */
  if (fchmod(STDIN_FILENO, TTY_PERM))
    fail(fchmod);
  
  /* Get TTY name for last part of this functions */
  tty_device = ttyname(STDIN_FILENO);
  
  /* Kill other processes on this TTY */
  tcgetattr(STDIN_FILENO, &tty);
  saved_tty = tty;
  tty.c_cflag &= ~HUPCL;
  tcsetattr(0, TCSANOW, &tty);
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  signal(SIGHUP, SIG_IGN);
  vhangup();
  signal(SIGHUP, SIG_DFL);
  
  /* Restore terminal and TTY modes */
  fd = open(tty_device, O_RDWR | O_NONBLOCK);
  if (fd == -1)
    fail(open);
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK);
  for (i = 0; i < fd; i++)
    close(i);
  for (i = 0; i < 3; i++)
    if (i != fd)
      dup2(fd, i);
  if (fd > 2)
    close(fd);
  tcgetattr(STDIN_FILENO, &saved_tty);
}

