#include <inttypes.h>
#include <getopt.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/ioctl.h>

#include <linux/uinput.h>
#include <linux/input.h>

#define UINPUT_DEV	"/dev/uinput"

enum input_action {
	LIST,
	FIND,
};

int input_devices_lookup(const char *input_name, enum input_action list)
{
	const char *dirname = "/dev/input";
	char devname[PATH_MAX];
	struct dirent *de;
	char *filename;
	DIR *dir;
	int fd = -1;

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

			switch (list) {
				case LIST:
					printf("[%s]\n", name);
					close(fd);
					break;
				case FIND:
					if (!strcmp(name, input_name)) {
						goto ready;
					} else {
						close(fd);
						fd = -1;
					}
					break;
				default:
					printf("%s: unknown action %d\n", __func__, list);
					return -1;
			}
		}
	}

ready:
	closedir(dir);
	return fd;
}

void usage(char *name)
{
	printf("%s [-e <input>] [-l] [-h]\n", name);
	printf("\t-d <input>: input device name\n");
	printf("\t-l: list input devices\n");
	printf("\t-h: this help message\n");
}

int main(int argc, char **argv)
{
	static struct uinput_user_dev uinput_dev;
	struct input_event event;
	int type, code, nread;
	int uinput_fd;
	float value;
	int i, opt;

	char *input_name = NULL;

	while ((opt = getopt(argc, argv, "lhe:")) != -1) {
		switch (opt) {
			case 'e':
				input_name = strdup(optarg);
				break;
			case 'l':
				input_devices_lookup(NULL, LIST);
				exit(0);
			case 'h':
			default:
				usage(argv[0]);
				exit(0);
		}
	}

	if (!input_name) {
		usage(argv[0]);
		exit(0);
	}

	if (0 > (uinput_fd = input_devices_lookup(input_name, FIND))) {
		printf("Can't open device named '%s'\n", input_name);
		exit(-1);
	}

	printf("start reading events...\n");

	while (1) {
		nread = read(uinput_fd, &event, sizeof(struct input_event));
		if (nread != sizeof(struct input_event)) {
			perror("read event");
			exit(-1);
		}

		type = event.type;

		if (type == EV_REL) {
			float value = event.value;
			if (event.code == REL_X) {
				printf("REL_X [%.2f]\n", value);
			} else if (event.code == REL_Y) {
				printf("REL_Y [%.2f]\n", value);
			} else if (event.code == REL_Z) {
				printf("REL_Z [%.2f]\n", value);
			}
		} else if (type == EV_ABS) {
			float value = event.value;
			if (event.code == ABS_X) {
				printf("ABS_X [%.2f]\n", value);
			} else if (event.code == REL_Y) {
				printf("ABS_Y [%.2f]\n", value);
			} else {
				printf("WARN: unknown ABS event\n");
			}
		} else if (type == EV_SYN) {
			printf("EV_SYN\n");
		} else if (type == EV_KEY) {
			printf("EV_KEY: %d\n", event.code);
		} else {
			printf("WARN: unknown event (type=%d, code=%d)\n",
					type, event.code);
		}

	}
}
