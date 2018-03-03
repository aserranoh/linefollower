
/* daemon.h
   Function to daemonize a process.
   
   Copyright 2018 Antonio Serrano Hernandez

   This file is part of linefollower.

   linefollower is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   linefollower is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with linefollower; see the file COPYING.  If not, see
   <http://www.gnu.org/licenses/>.
*/

#ifndef DAEMON_H
#define DAEMON_H

#include <err.h>            // warn
#include <stdlib.h>         // exit
#include <fcntl.h>          // open
#include <signal.h>         // sigaction
#include <string.h>         // strlen
#include <sys/resource.h>   // getrlimit
#include <sys/stat.h>       // umask
#include <unistd.h>         // fork

/* Daemonize this process.

   Paremters:
     * pidfile: the path to the pidfile to be created, or NULL if no pidfile
         has to be created.

   Return 0 if the process is correctly daemonized, 1 in case of error.
*/
int
daemonize(const char *pidfile)
{
    int i, fd0, fd1, fd2, pidfd;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;
    char buf[16];

    // Clear file creation mask.
    umask(0);

    // Get maximum number of file descriptors.
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        warn("can't get file limit");
        return 1;
    }

    // Become a session leader to lose controlling TTY.
    if ((pid = fork()) < 0) {
        warn("can't fork");
        return 1;
    } else if (pid != 0) {
        // Parent
        exit(0);
    }
    setsid();

    // Ensure future opens won't allocate controlling TTYs.
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0) {
        warn("can't ignore SIGHUP");
        return 1;
    }
    if ((pid = fork()) < 0) {
        warn("can't fork");
        return 1;
    } else if (pid != 0) {
        // Parent
        exit(0);
    }

    // Change the current working directory to the root so we won't prevent
    // file systems from being unmounted.
    if (chdir("/") < 0) {
        warn("can't change directory to /");
        return 1;
    }

    // Create the pidfile
    if (pidfile) {
        pidfd = open(
            pidfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (pidfd < 0) {
            warn("can't create the pidfile");
            return 1;
        }
        if (ftruncate(pidfd, 0) < 0) {
            warn("can't write to the pidfile");
            return 1;
        }
        sprintf(buf, "%ld", (long)getpid());
        if (write(pidfd, buf, strlen(buf)) < 0) {
            warn("can't write to the pidfile");
            return 1;
        }
    }

    // Close all open file descriptors.
    if (rl.rlim_max == RLIM_INFINITY) {
        rl.rlim_max = 1024;
    }
    for (i = 0; i < rl.rlim_max; i++) {
        close(i);
    }

    // Attach file descriptors 0, 1, and 2 to /dev/null.
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);
    return 0;
}

#endif

