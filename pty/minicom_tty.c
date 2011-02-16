#include <sys/types.h>
#include <termios.h>
#include <strings.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <utmp.h>
#include <pty.h>

#define STR_LEN 100
#define SPEED 	B115200

void sig_pipe(int signo);

int main(void)
{
	char fds_name[20], line[STR_LEN];
	int max, fdm, fds, tty, rc;
	struct termios trm, ts;
	struct sigaction act;
	struct timeval tv;
	fd_set rset;
	pid_t pid;

	bzero(&act,sizeof(act));

	/* setup SIGPIPE handler */
	act.sa_handler = sig_pipe;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGPIPE,&act,NULL);

	/* create and configure pty */

	if ((rc = openpty(&fdm, &fds, fds_name, &trm, NULL)) < 0) {
		perror("openpty");
		exit(-1);
	}
	
	printf("master ->  %s[%d]   slave -> %s[%d]\n", 
		ttyname(fdm), fdm, fds_name, fds);

	cfmakeraw(&ts);  
	cfsetispeed(&ts, SPEED);
	cfsetospeed(&ts, SPEED);
	tcsetattr(fdm, TCSANOW, &ts); 
	tcsetattr(fds, TCSANOW, &ts); 

	/* open and configure serial port */

	if ((tty = open("/dev/ttyUSB0", O_RDWR)) < 0) {
		perror("open ttyUSB0");
		exit(-1);
	}

	cfmakeraw(&ts);  
	cfsetispeed(&ts, SPEED);
	cfsetospeed(&ts, SPEED);
	tcsetattr(tty, TCSANOW, &ts); 

	/* select cycle */
	
	while(1) {

		FD_ZERO(&rset);
		FD_SET(fdm, &rset);
		FD_SET(tty, &rset);
		
		max = fdm > tty ? fdm : tty;

		tv.tv_sec = 5;
		tv.tv_usec = 0;

		rc = select(max + 1, &rset, NULL, NULL, &tv);

		if(rc == 0){
			/* printf("select timeout\n"); */
			continue;
		}

		if(rc == -1){
			if(errno == EINTR){
				perror("'select' was interrupted, try again");
				continue;
			}else{
				perror("exit - some other select problem");
				exit(1);
			}
		}

		if (FD_ISSET(fdm, &rset)){		

			rc = read(fdm, line, sizeof(line));

			if (0 > rc) {
				perror("read from  master pty");
				exit(-1);
			}

			printf("read %d symbols from pty: %s\n", rc, line);

			rc = write(tty, line, strlen(line));

			if (0 > rc) {
				perror("write to tty");
				exit(-1);
			}

			printf("wrote %d symbols from %d to tty: %s", rc, strlen(line), line);
			bzero(line, sizeof(line));

		} else if (FD_ISSET(tty, &rset)) {

			rc = read(tty, line, sizeof(line));

			if (0 > rc) {
				perror("read from  tty");
				exit(-1);
			}

			printf("read %d symbols from tty: %s\n", rc, line);

			rc = write(fdm, line, strlen(line));

			if (0 > rc) {
				perror("write to master pty");
				exit(-1);
			}

			printf("wrote %d symbols from %d to pty: %s", rc, strlen(line), line);
			bzero(line, sizeof(line));

		} else {
			fprintf(stderr, "shouldn't be here: no data for known descriptors\n");
			continue;
		}

	}

	return 0;
}

void sig_pipe(int signo)
{
	printf("parent got SIGPIPE\n");
	fflush(stdout);
}
