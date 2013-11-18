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


static inline void fail(char* str)
{
  perror(str);
  sleep(FAILURE_SLEEP);
  _exit(1);
}


/**
 * Secure the TTY from spying
 * 
 * @param  group  The group, -1 for unchanged
 */
void secure_tty(gid_t group)
{
  struct termios tty;
  struct termios saved_tty;
  char* tty_device;
  int fd, i;
  
  /* Set ownership of this TTY to root:root */
  chown_tty(0, group, 1);
  
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
    fail("open");
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


/**
 * Set ownership and mode of the TTY
 * 
 * @param  owner      The owner, -1 for unchanged
 * @param  group      The group, -1 for unchanged
 * @param  with_fail  Abort on failure
 */
void chown_tty(uid_t owner, gid_t group, int with_fail) 
{
  #if defined(OWN_VCSA) || defined(OWN_VCS)
  struct vt_stat vtstat;
  #endif
  
  /* Set ownership of this TTY */
  if (fchown(STDIN_FILENO, owner, group) && with_fail)
    fail("fchown");
  
  /* Restrict others from using this TTY */
  if (fchmod(STDIN_FILENO, TTY_PERM) && with_fail)
    fail("fchmod");
  
  /* Also do the above for /dev/vcs[a][0-9]+ */
  #if defined(OWN_VCSA) || defined(OWN_VCS)
    if (ioctl(STDIN_FILENO, VT_GETSTATE, &vtstat) == 0)
      {
	int n = vtstat.v_active;
	char _vcs[16];
	char _vcsa[16];
	
	char* vcs = _vcs;
	char* vcsa = _vcsa;
	vcs += 16;
	vcsa += 16;
	
	if (n)
	  {
	    *--vcs = *--vcsa = 0;
	    while (n)
	      {
		*--vcs = *--vcsa = (n % 10) + '0';
		n /= 10;
	      }
	    
	    vcs -= 8;
	    vcsa -= 9;
	    strcpy(vcs,  "/dev/vcs");
	    strcpy(vcsa, "/dev/vcsa");
	    
	    #ifdef OWN_VCS
	      if (chown(vcs,  owner, group) && with_fail)  fail("chown");
	      if (chmod(vcs,  TTY_PERM) && with_fail)  fail("chmod");
	    #endif
	    #ifdef OWN_VCSA
	      if (chown(vcsa, owner, group) && with_fail)  fail("chown");
	      if (chmod(vcsa, TTY_PERM) && with_fail)  fail("chmod");
	    #endif
	  }
      }
  #endif
}

