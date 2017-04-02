#include <sys/inotify.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char **argv)
{
	char events[4096] __attribute__ ((aligned(4)));
	const struct inotify_event *event;
	fd_set rset;
	int ifd;
	int wfd;
	int len;
	int ret;
	int i;
	int j;

	if (argc != 2) {
		printf("Usage: %s </path/to/dir/or/file> \n", argv[0]);
		exit(-1);
	}

	/* init inotify */

	ifd = inotify_init1(0);
	if (ifd < 0) {
		perror("inotify_init1");
		exit(-1);
	}

	/* add inotify watch */

	wfd = inotify_add_watch(ifd, argv[1], IN_ALL_EVENTS);
	if (wfd < 0) {
		perror("inotify_add_watch");
		exit(-1);
	}

	/* wait for events */

	while(1) {

		FD_SET(ifd, &rset);

		ret = select(ifd + 1, &rset, NULL, NULL, NULL);
		if (ret < 0) {
			if(errno == EINTR){
				perror("select interrupt");
				exit(0);
			} else {
				perror("select error");
				exit(-1);
			}
		}

		if (FD_ISSET(ifd, &rset)) {

			/* check event queue length */

			ret = ioctl(ifd, FIONREAD, &len);
			if (ret < 0) {
				perror("ioctl");
				exit(-1);
			} else {
				printf("INOTIFY: pending %d events\n", len / sizeof(*event));
			}

			/* read inotify events */

			len = read(ifd, events, sizeof(events));
			if (len < 0) {
				perror("read inotify events");
				exit(-1);
			}

			if(len == 0) {
				perror("zero read");
				continue;
			}

			i = j = 0;

			while (i < len) {
				event = (struct inotify_event *)&events[i];
				i += sizeof(*event) + event->len;

				printf("%5sINOTIFY: event #%d\n", "", ++j);
				printf("%10swd(%d): mask(0x%08x): %s%s%s%s%s%s%s\n", "",
					event->wd, event->mask,
					(event->mask & IN_ISDIR) ? "ISDIR ": "",
					(event->mask & IN_ACCESS) ? "ACCESS ": "",
					(event->mask & IN_OPEN) ? "OPEN ": "",
					(event->mask & IN_MODIFY) ? "MODIFY ": "",
					(event->mask & IN_CREATE) ? "CREATE ": "",
					(event->mask & IN_DELETE) ? "DELETE ": "",
					(event->mask & IN_CLOSE) ? "CLOSE ": "");
			}
		}
	}

	ret = inotify_rm_watch(ifd, wfd);
	if (ret < 0)
		perror("inotify_rm_watch");

	ret = close(ifd);
	if (ret < 0)
		perror("close inotify");
}
