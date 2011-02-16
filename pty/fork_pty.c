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

void parent_sigchld(int signo);
void parent_sigpipe(int signo);
void child_sigint(int signo);
		
FILE *logger;

int main(void)
{
	char fds_name[20], line[STR_LEN];
	struct termios trm, ts;
	struct sigaction act;
	int fdm, rc;
	pid_t pid;

	bzero(&act,sizeof(act));

	if ((pid = forkpty(&fdm, fds_name, &trm, NULL)) < 0) {
		perror("forkpty");
		exit(-1);
	}

	if (pid == 0) {		/* child */
		
		char ch;

		/* open logger file */
		logger = fopen("./chld.logger", "w+");
		fprintf(logger, "logger activated\n", logger);
		fflush(logger);
		
		/* setup SIGINT handler */
		act.sa_handler = child_sigint;
		sigemptyset(&act.sa_mask);
		act.sa_flags = 0;
		sigaction(SIGINT,&act,NULL);
		
		/* set canonic mode for slave pty */
		tcgetattr(fileno(stdin), &ts); 

		cfsetspeed(&ts, B19200);

		ts.c_lflag &=  ~(ECHO | ECHOE | ECHOK | ECHONL);
		ts.c_lflag |= ICANON | IEXTEN | ISIG;

		ts.c_iflag &= ~(IGNBRK | ISTRIP | INLCR | IGNCR);
		ts.c_iflag |= (BRKINT | ICRNL);

		ts.c_cc[VINTR] = 0x03; /* ^C for SIGINT */
		ts.c_cc[VEOF] = 0x04; /* ^D for EOF */

		tcsetattr(fileno(stdin), TCSANOW, &ts);

		while (1) {
			
			bzero(line, sizeof(line));

			if (NULL == fgets(line, sizeof(line), stdin)) {
				fprintf(logger, "got NULL from slave pty\n");
				break;
			}

			fputs(line, logger);
			fflush(logger);
			
			if (EOF == fputs(line, stdout)) {
				fprintf(logger, "puts to slave pty returned NULL\n");
				break;
			}

			/*
			rc = read(fileno(stdin), &ch, 1);
			
			if (rc < 0) {
				fprintf(logger, "error: slave pty read\n");
				fflush(logger);
				break;
			}
			
			if (rc > 0) {
				fprintf(logger, "read %d bytes of data: %d\n", rc, ch);
				fflush(logger);
			}
			
			if (rc == 0) {
				continue;
			}

			rc = write(fileno(stdout), &ch, 1);
	
			if (rc < 0) {
				fprintf(logger, "error: slave pty write\n");
				fflush(logger);
				break;
			}
			
			if (rc > 0) {
				fprintf(logger, "wrote %d bytes of  data: %d\n", rc, ch);
				fflush(logger);
			}
			*/
		}
		
		fprintf(logger, "completed...\n");
		fflush(logger);
		fclose(logger);
	}

	/* parent */

	printf("master ->  %s   slave -> %s\n", ttyname(fdm), fds_name);
	
	/* setup SIGCHLD handler */	
	act.sa_handler = parent_sigchld;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGCHLD,&act,NULL);
	
	/* setup SIGPIPE handler */
	act.sa_handler = parent_sigpipe;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGPIPE,&act,NULL);

	/* echo off for master pty */
	tcgetattr(fdm, &ts);  
	ts.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL); 
	tcsetattr(fdm, TCSANOW, &ts); 

	/* read/write cycle */
	
	while(1) {
		
		bzero(line, sizeof(line));
		printf("enter string:\n");

		if (NULL == fgets(line, sizeof(line), stdin)) {
			printf("EOF\n");
			break;
		}

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

		rc = write(fdm, line, strlen(line));

		if (0 > rc) {
			perror("write to master pty");
			exit(-1);
		}
		
		printf("wrote %d symbols from %d to pty: %s", rc, strlen(line), line);
		bzero(line, sizeof(line));

		rc = read(fdm, line, sizeof(line));

		if (0 > rc) {
			perror("read from  master pty");
			exit(-1);
		}

		printf("read %d symbols from pty: %s\n", rc, line);

	}

	return 0;
}

void parent_sigpipe(int signo)
{
	printf("parent got SIGPIPE\n");
	fflush(stdout);
}

void parent_sigchld(int signo)
{
	printf("parent got SIGCHLD\n");
	fflush(stdout);
}

void child_sigint(int signo)
{
	fprintf(logger, "child got SIGINT\n");
	fflush(logger);
}
