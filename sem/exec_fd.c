#include <semaphore.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>

#define CYCLE 5
#define SEM_NAME_A "my_sem_a"
#define SEM_NAME_B "my_sem_b"

/*
 *  File descriptor is created, then process forks.
 *  Parent and child one by one read from the same descriptor.
 *  
 */

void int_handler(int, siginfo_t *, void *);

int main(int argc, char ** argv){

	int i = 0, j = 0;

	sem_t * lock_a, * lock_b;
	struct sigaction act;
	sigset_t sig;
	pid_t pid;
	int fd;
	char c;

	/* block SIGCHLD */
	sigemptyset(&sig);
	sigaddset(&sig, SIGCHLD);
	sigprocmask(SIG_BLOCK, &sig, NULL);

	/* set SIGINT handler */
	act.sa_sigaction = int_handler;
	act.sa_flags = SA_SIGINFO;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGCHLD);
	sigaction(SIGINT, &act, NULL);

	/* create shared semaphore and shared file descriptor */
	fd = open("exec_fd.c", O_RDONLY);

	if (0 > fd) {
		perror("open");
		exit(1);
	}

	lock_a = sem_open(SEM_NAME_A, O_CREAT, O_RDWR, 1);
	lock_b = sem_open(SEM_NAME_B, O_CREAT, O_RDWR, 0);

	if (lock_a == SEM_FAILED || lock_b == SEM_FAILED) {
		perror("sem_open");
		exit(1);
	}

	/* fork and read from file in both parent and child */
	pid = fork();

	if (0 > pid) {
		perror("fork");
		exit(1);
	}

	if (0 == pid) {
		/* child */

		/* block SIGINT */
		sigemptyset(&sig);
		sigaddset(&sig, SIGINT);
		sigprocmask(SIG_BLOCK, &sig, NULL);

		for(i = 0; i < CYCLE; i++){
			sem_wait(lock_a);
			read(fd, &c, 1);
			printf(">>> child got: %c\n", c);
			sleep(1);
			sem_post(lock_b);
		}

		if (0 > sem_close(lock_a)) {
			perror("sem_close");
		}

		if (0 > sem_close(lock_b)) {
			perror("sem_close");
		}

		close(fd);
		exit(0);
	}

	/* parent */
	for(j = 0; j < CYCLE; j++){
		sem_wait(lock_b);
		read(fd, &c, 1);
		printf(">>> parent got: %c\n", c);
		sleep(1);
		sem_post(lock_a);
	}

	/* delete semaphore */
	if (0 > sem_unlink(SEM_NAME_A)) {
		perror("sem_unlink");
	}

	if (0 > sem_unlink(SEM_NAME_B)) {
		perror("sem_unlink");
	}

	close(fd);
	exit(0);
}

void int_handler(int signo, siginfo_t * s, void * v)
{
	if (0 > sem_unlink(SEM_NAME_A)) {
		perror("sem_unlink");
	}

	if (0 > sem_unlink(SEM_NAME_B)) {
		perror("sem_unlink");
	}

	exit(0);
}
