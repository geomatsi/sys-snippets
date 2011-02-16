#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#define HOME "/path/to/daemon/new/home/dir"

void sighup_handler(int sig)
{
	(void) sig;
	printf("got SIGHUP\n");
	fflush(stdin);
}

int main(void)
{
	int log, null;
	pid_t pid;
	int i;

	/* daemonize process */

	if (0 != (pid = fork())) {
		exit(0);
	}

	if ( -1 == setsid()) {
		perror("setsid problem");
		exit(-1);
	}

	signal(SIGHUP, sighup_handler);

	if (0 != (pid = fork())) {
		exit(0);
	}

	chdir(HOME);

	if ( 0 > (log = open("test.log", O_RDWR | O_CREAT | O_APPEND, S_IWUSR | S_IRUSR))) {
		perror("open log file");
		exit(-1);
	}

	if ( 0 > (null = open("/dev/null", O_RDONLY))) {
		perror("open /dev/null");
		exit(-1);
	}

	dup2(log, fileno(stdout));
	dup2(log, fileno(stderr));
	dup2(null, fileno(stdin));

	for(i = 0; i < 10; i++){
		printf("cycle %d\n", i);
		fflush(stdout);
		sleep(1);
	}

	printf("stopping daemon...\n");
	fflush(stdout);
	close(null);
	close(log);
	exit(0);
}

