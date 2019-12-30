// SPDX-License-Identifier: GPL-2.0

#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#define HOME "/tmp"

void sighup_handler(int sig)
{
	printf("SIGHUP\n");
	fflush(stdin);
}

int main(void)
{
	pid_t pid;
	int null;
	int log;
	int ret;
	int i;

	pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(-1);
	}

	if (pid != 0)
		exit(0);

	ret = setsid();
	if (ret < 0) {
		perror("setsid");
		exit(-1);
	}

	signal(SIGHUP, sighup_handler);

	pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(-1);
	}

	if (pid != 0)
		exit(0);

	ret = chdir(HOME);
	if (ret < 0)
		perror("chdir");

	log = open("test.log", O_RDWR | O_CREAT | O_APPEND, S_IWUSR | S_IRUSR);
	if (log < 0) {
		perror("open log");
		exit(-1);
	}

	null = open("/dev/null", O_RDONLY);
	if (null < 0) {
		perror("open /dev/null");
		exit(-1);
	}

	ret = dup2(log, fileno(stdout));
	if (ret < 0) {
		perror("dup2 stdout");
		exit(-1);
	}

	ret = dup2(log, fileno(stderr));
	if (ret < 0) {
		perror("dup2 stderr");
		exit(-1);
	}

	ret = dup2(null, fileno(stdin));
	if (ret < 0) {
		perror("dup2 stdin");
		exit(-1);
	}

	for (i = 0; i < 30; i++) {
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
