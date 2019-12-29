// SPDX-License-Identifier: GPL-2.0

#include <linux/usbdevice_fs.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
	const char *name;
	int fd, ret;

	if (argc < 2) {
		printf("Usage: usbreset <devname>\n");
		return 0;
	}

	name = argv[1];
	fd = open(name, O_WRONLY);
	if (fd < 0) {
		perror("open");
		return -1;
	}

	printf("reset USB device %s\n", name);

	ret = ioctl(fd, USBDEVFS_RESET, 0);
	if (ret < 0) {
		perror("ioctl");
		return -1;
	}

	close(fd);
	return 0;
}
