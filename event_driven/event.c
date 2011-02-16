#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdint.h> 
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

/*
#include <sys/timerfd.h>
#include <sys/eventfd.h>
*/

#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

/* if eventfd/timerfd features are not implemented in glibc, then we use syscalls directly */

#define TFD_TIMER_ABSTIME (1 << 0)

#if defined(__i386__)

#define __NR_timerfd_create 322
#define __NR_timerfd_settime 325
#define __NR_timerfd_gettime 326

#elif defined(__arm__)

#define __NR_timerfd_create  350
#define __NR_timerfd_settime 353
#define __NR_timerfd_gettime 354

#endif

static long timerfd_create(int clockid, int flag)
{
	return syscall(__NR_timerfd_create, clockid, flag);
}

static long timerfd_settime(int fd, int flags, const struct itimerspec *new_value, struct itimerspec *curr_value)
{
	return syscall(__NR_timerfd_settime, fd, flags, new_value, curr_value);
}

static long timerfd_gettime(int fd, struct itimerspec *curr_value)
{
	return syscall(__NR_timerfd_gettime, fd, curr_value);
}

#if defined(__i386__)

#define __NR_eventfd 323

static int eventfd(int count, int flag) 
{
	return syscall(__NR_eventfd, count, flag);
}

#elif defined(__arm__)
#include <sys/eventfd.h>
#else 
#error unknown architecture
#endif

/* */

void int_handler(int);

/* */

int main(void)
{
	struct itimerspec new_value;
	struct sigaction act;
	struct timespec now;
	struct timeval tv;
	uint64_t exp;
	int efd, tfd, pfd;

	fd_set rset;
	int rc, max;

	/* setup signal handler */

	bzero(&act,sizeof(act));
	act.sa_handler = int_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT,&act,NULL);

	/* init eventfd */	

	if (0 > (efd = eventfd(0, 0))) {
		perror("eventfd");
		exit(-1);
	}

	/* init timerfd */

	memset(&new_value, 0x0, sizeof(new_value));
	memset(&now, 0x0, sizeof(now));

	if (0 > (tfd = timerfd_create(CLOCK_MONOTONIC, 0))) {
		perror("timerfd_create");
		exit(-1);
	}

	if (clock_gettime(CLOCK_MONOTONIC, &now) == -1) {
		perror("clock_gettime");
		exit(-1);
	}

	new_value.it_value.tv_sec = (long) (now.tv_sec + 5);

	if (0 > timerfd_settime(tfd, TFD_TIMER_ABSTIME, &new_value, NULL)) {
		perror("timerfd_settime");
		exit(-1);
	}

	memset(&new_value, 0x0, sizeof(new_value));

	if (0 > timerfd_gettime(tfd, &new_value)) {
		perror("timerfd_gettime");
		exit(-1);
	}

	printf("timer expires in %lu sec\n", (long) new_value.it_value.tv_sec);

	/* init periodic timerfd */

	memset(&new_value, 0x0, sizeof(new_value));
	memset(&now, 0x0, sizeof(now));

	if (0 > (pfd = timerfd_create(CLOCK_MONOTONIC, 0))) {
		perror("periodic timerfd_create");
		exit(-1);
	}

	if (clock_gettime(CLOCK_MONOTONIC, &now) == -1) {
		perror("periodic clock_gettime");
		exit(-1);
	}

	new_value.it_value.tv_sec = (long) (now.tv_sec + 3);
	new_value.it_interval.tv_sec = (long) (7);

	if (0 > timerfd_settime(pfd, TFD_TIMER_ABSTIME, &new_value, NULL)) {
		perror("periodic timerfd_settime");
		exit(-1);
	}

	memset(&new_value, 0x0, sizeof(new_value));

	if (0 > timerfd_gettime(pfd, &new_value)) {
		perror("periodic timerfd_gettime");
		exit(-1);
	}

	printf("at timer expires in %lu sec, then each %lu sec\n", 
			(long) new_value.it_value.tv_sec, (long) new_value.it_interval.tv_sec);

	/* select loop */

	for(;;){

		FD_ZERO(&rset);

		FD_SET(efd, &rset);
		FD_SET(tfd, &rset);
		FD_SET(pfd, &rset);

		max = efd > tfd ? efd : tfd;
		max = pfd > max ? pfd : max;

		tv.tv_sec = 1;
		tv.tv_usec = 0;

		rc = select(max + 1, &rset, NULL, NULL, &tv);

		if (rc == 0) {
			printf("select timeout\n");			
			continue;
		}

		if (rc == -1) {
			if (errno == EINTR) {
				printf("select interrupted by signal, try again\n");
				continue;
			}else{
				perror("select");
				exit(-1);
			}
		}

		if (FD_ISSET(tfd, &rset)) {		
			rc = read(tfd, &exp, sizeof(uint64_t));

			if (rc != sizeof(uint64_t)) {
				perror("timer read");
				exit(-1);
			} 

			memset(&new_value, 0x0, sizeof(new_value));
			memset(&now, 0x0, sizeof(now));

			if (clock_gettime(CLOCK_MONOTONIC, &now) == -1) {
				perror("clock_gettime");
				exit(-1);
			}

			new_value.it_value.tv_sec = (long) (now.tv_sec + 5);

			if (0 > timerfd_settime(tfd, TFD_TIMER_ABSTIME, &new_value, NULL)) {
				perror("timerfd_settime");
				exit(-1);
			}

			printf("timerfd expired %llu times\n", exp);
			rc = write(efd, &exp, sizeof(uint64_t));

			if (rc < 0) {
				perror("eventfd write");
				exit(-1);
			} 

		} else if (FD_ISSET(efd, &rset)) {		
			rc = read(efd, &exp, sizeof(uint64_t));

			if (rc != sizeof(uint64_t)) {
				perror("eventfd read");
				exit(-1);
			} 

			printf("eventfd: %llu times\n", exp);

		} else if (FD_ISSET(pfd, &rset)) {		
			rc = read(pfd, &exp, sizeof(uint64_t));

			if (rc != sizeof(uint64_t)) {
				perror("periodic timer read");
				exit(-1);
			} 

			printf("periodic timerfd expired %llu times\n", exp);

		} else {
			printf("SHOULDN'T HAPPEN !!!\n");
		}
	}
}

void int_handler(int t)
{
	(void)t;

	printf("SIGINT...\n");
	exit(0);
}
