// SPDX-License-Identifier: GPL-2.0

#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

static void sigint_handler(int);

int main(void)
{
	struct itimerspec new_value;
	struct sigaction act;
	struct timespec now;
	struct timeval tv;
	uint64_t exp;
	fd_set rset;
	int efd;
	int tfd;
	int pfd;
	int ret;
	int max;

	memset(&act, 0x0, sizeof(act));
	act.sa_handler = sigint_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	sigaction(SIGINT, &act, NULL);

	efd = eventfd(0, 0);
	if (efd < 0) {
		perror("eventfd");
		exit(-1);
	}

	memset(&new_value, 0x0, sizeof(new_value));
	memset(&now, 0x0, sizeof(now));

	tfd = timerfd_create(CLOCK_MONOTONIC, 0);
	if (tfd < 0) {
		perror("timerfd_create(tfd)");
		exit(-1);
	}

	ret = clock_gettime(CLOCK_MONOTONIC, &now);
	if (ret < 0) {
		perror("clock_gettime");
		exit(-1);
	}

	new_value.it_value.tv_sec = (long) (now.tv_sec + 5);

	ret = timerfd_settime(tfd, TFD_TIMER_ABSTIME, &new_value, NULL);
	if (ret < 0) {
		perror("timerfd_settime(tfd)");
		exit(-1);
	}

	memset(&new_value, 0x0, sizeof(new_value));

	ret = timerfd_gettime(tfd, &new_value);
	if (ret < 0) {
		perror("timerfd_gettime(tfd)");
		exit(-1);
	}

	printf("single shot timer expires in %lu sec\n", (long)new_value.it_value.tv_sec);

	memset(&new_value, 0x0, sizeof(new_value));
	memset(&now, 0x0, sizeof(now));

	pfd = timerfd_create(CLOCK_MONOTONIC, 0);
	if (pfd < 0) {
		perror("timerfd_create(pfd)");
		exit(-1);
	}

	ret = clock_gettime(CLOCK_MONOTONIC, &now);
	if (ret < 0) {
		perror("clock_gettime");
		exit(-1);
	}

	new_value.it_value.tv_sec = (long)(now.tv_sec + 3);
	new_value.it_interval.tv_sec = 7L;

	ret = timerfd_settime(pfd, TFD_TIMER_ABSTIME, &new_value, NULL);
	if (ret < 0) {
		perror("timerfd_settime(pfd)");
		exit(-1);
	}

	memset(&new_value, 0x0, sizeof(new_value));

	ret = timerfd_gettime(pfd, &new_value);
	if (ret < 0) {
		perror("timerfd_gettime(pfd)");
		exit(-1);
	}

	printf("periodic timer expires in %lu sec, then each %lu sec\n",
	       (long) new_value.it_value.tv_sec, (long) new_value.it_interval.tv_sec);

	for (;;) {
		FD_ZERO(&rset);
		FD_SET(efd, &rset);
		FD_SET(tfd, &rset);
		FD_SET(pfd, &rset);

		max = efd > tfd ? efd : tfd;
		max = pfd > max ? pfd : max;

		tv.tv_sec = 1;
		tv.tv_usec = 0;

		ret = select(max + 1, &rset, NULL, NULL, &tv);

		if (ret == 0) {
			printf("select timeout\n");
			continue;
		}

		if (ret < 0) {
			if (errno == EINTR) {
				printf("select interrupt\n");
				continue;
			} else {
				perror("select");
				exit(-1);
			}
		}

		if (FD_ISSET(tfd, &rset)) {
			ret = read(tfd, &exp, sizeof(uint64_t));
			if (ret != sizeof(uint64_t)) {
				perror("read(tfd)");
				exit(-1);
			}

			memset(&new_value, 0x0, sizeof(new_value));
			memset(&now, 0x0, sizeof(now));
			ret = clock_gettime(CLOCK_MONOTONIC, &now);
			if (ret < 0) {
				perror("clock_gettime");
				exit(-1);
			}

			new_value.it_value.tv_sec = (long)(now.tv_sec + 5);
			ret = timerfd_settime(tfd, TFD_TIMER_ABSTIME, &new_value, NULL);
			if (ret < 0) {
				perror("timerfd_settime(tfd)");
				exit(-1);
			}

			printf("timerfd expired %lu times\n", exp);
			ret = write(efd, &exp, sizeof(uint64_t));
			if (ret < 0) {
				perror("write(efd)");
				exit(-1);
			}
		} else if (FD_ISSET(efd, &rset)) {
			ret = read(efd, &exp, sizeof(uint64_t));
			if (ret != sizeof(uint64_t)) {
				perror("read(efd)");
				exit(-1);
			}

			printf("eventfd: %lu times\n", exp);

		} else if (FD_ISSET(pfd, &rset)) {
			ret = read(pfd, &exp, sizeof(uint64_t));
			if (ret != sizeof(uint64_t)) {
				perror("read(pfd)");
				exit(-1);
			}

			printf("periodic timerfd expired %lu times\n", exp);
		} else {
			assert(0);
		}
	}
}

void sigint_handler(int t)
{
	printf("SIGINT...\n");
	exit(0);
}
