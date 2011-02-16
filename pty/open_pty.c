#include <sys/types.h>
#include <termios.h>
#include <strings.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <utmp.h>
#include <pty.h>

#define STR_LEN 100

void sig_pipe(int signo);
		
FILE *logger;

int main(void)
{
	char fds_name[20], line[STR_LEN];
	int max, fdm, fds, std, rc;
	struct termios trm, ts;
	struct sigaction act;
	struct timeval tv;
	fd_set rset;
	pid_t pid;

	bzero(&act,sizeof(act));

	if ((rc = openpty(&fdm, &fds, fds_name, &trm, NULL)) < 0) {
		perror("openpty");
		exit(-1);
	}

	std = fileno(stdin);

	printf("master ->  %s[%d]   slave -> %s[%d]\n", 
		ttyname(fdm), fdm, fds_name, fds);

	/* setup SIGPIPE handler */
	act.sa_handler = sig_pipe;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGPIPE,&act,NULL);

	/* configure raw mode for both master and slave */
	cfmakeraw(&ts);  
	tcsetattr(fdm, TCSANOW, &ts); 
	tcsetattr(fds, TCSANOW, &ts); 

	/* select cycle */
	
	while(1) {

		FD_ZERO(&rset);
		FD_SET(fdm, &rset);
		FD_SET(std, &rset);
		//FD_SET(fds, &rset);
		
		max = fdm > std ? fdm : std;

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
			bzero(line, sizeof(line));

		//} else if (FD_ISSET(fds, &rset)){		

		} else if (FD_ISSET(std, &rset)) {

			rc = read(std, line, sizeof(line));

			if (0 > rc) {
				perror("read from  stdin");
				exit(-1);
			}

			printf("read %d symbols from stdin: %s\n", rc, line);

			rc = write(fdm, line, strlen(line));

			if (0 > rc) {
				perror("write to master pty");
				exit(-1);
			}

			printf("wrote %d symbols from %d to pty: %s", rc, strlen(line), line);
			bzero(line, sizeof(line));

			/*
			if (0 == strncmp(line, "int", 3)) {
				int ctrl_c = 0x03;
				printf("send ^C to slave pty\n");
				write(fdm, &ctrl_c, 1);
				break;
			}

			if (0 == strncmp(line, "eof", 3)) {
				int ctrl_d = 0x04;
				printf("send ^D to slave pty\n");
				write(fdm, &ctrl_d, 1);
				break;
			}
			*/
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
