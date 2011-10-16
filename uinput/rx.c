#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <inttypes.h>

#include <sys/types.h>
#include <sys/ioctl.h>

#include <linux/uinput.h>
#include <linux/input.h>

#define LOGE		printf
#define UINPUT_DEV	"/dev/uinput"

int open_input(const char* inputName)
{
	int fd = -1;
	const char *dirname = "/dev/input";
	char devname[PATH_MAX];
	char *filename;
	DIR *dir;
	struct dirent *de;

	dir = opendir(dirname);
	if(dir == NULL)
		return -1;
	strcpy(devname, dirname);
	filename = devname + strlen(devname);
	*filename++ = '/';

	while((de = readdir(dir))) {
		if(de->d_name[0] == '.' &&
				(de->d_name[1] == '\0' ||
				 (de->d_name[1] == '.' && de->d_name[2] == '\0')))
			continue;
		strcpy(filename, de->d_name);
		fd = open(devname, O_RDONLY);
		if (fd>=0) {
			char name[80];
			if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
				name[0] = '\0';
			}
			if (!strcmp(name, inputName)) {
				break;
			} else {
				close(fd);
				fd = -1;
			}
		}
	}

	closedir(dir);
	return fd;
}

int main(void)
{
	static struct uinput_user_dev uinput_dev;
	struct input_event event;
	int type, code, nread;
	int uinput_fd;
	int i;
	float value;

	if (0 > (uinput_fd = open_input("uinput_test"))) {
		LOGE("Can't open device named 'uinput_test'\n");
		exit(-1);
	}


	LOGE("start reading events...\n");

	while (1) {
		nread = read(uinput_fd, &event, sizeof(struct input_event));
		if (nread != sizeof(struct input_event)) {
			perror("read event13");
			exit(-1);
		}

		type = event.type;

		if (type == EV_REL) {
			float value = event.value;
			if (event.code == REL_X) {
				LOGE("ACCEL_X [%.2f]\n", value);
			} else if (event.code == REL_Y) {
				LOGE("ACCEL_Y [%.2f]\n", value);
			} else if (event.code == REL_Z) {
				LOGE("ACCEL_Z [%.2f]\n", value);
			}
		} else if (type == EV_SYN) {
			LOGE("EV_SYN\n");
		} else {
			LOGE("AccelSensor: unknown event (type=%d, code=%d)",
					type, event.code);
		}

	}
}
