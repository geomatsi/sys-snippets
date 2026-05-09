#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#include <sys/types.h>
#include <sys/ioctl.h>

#include <linux/uinput.h>
#include <linux/input.h>

#include <wii_acc.h>

#ifdef ANDROID

#define LOG_TAG "wiid_acc"
#define LOG_NDEBUG 0
#include <cutils/log.h>

#else

#include <wii_log.h>

#endif /* ANDROID */

#define UINPUT_DEV	"/dev/uinput"

static struct uinput_user_dev uinput_dev;
static unsigned long long acc_time_msec;
static int acc_rate_msec = 100;
static int uinput_fd = -1;

/* */

int wii_acc_init(void)
{
	int i, ret;

	uinput_fd = open(UINPUT_DEV, O_RDWR);
	if (uinput_fd < 0) {
		LOGE("Error opening %s\n", UINPUT_DEV);
		ret = -EINVAL;
		goto open_err;
	}

	if (0 > ioctl(uinput_fd, UI_SET_EVBIT, EV_REL)) {
		LOGE("ioctl(fd, UI_SET_EVBIT, EV_REL)\n");
		ret = -EINVAL;
		goto ioctl_err;
	}

	if (0 > ioctl(uinput_fd, UI_SET_RELBIT, REL_X)) {
		LOGE("ioctl(fd, UI_SET_RELBIT, REL_X)\n");
		ret = -EINVAL;
		goto ioctl_err;
	}

	if (0 > ioctl(uinput_fd, UI_SET_RELBIT, REL_Y)) {
		LOGE("ioctl(fd, UI_SET_RELBIT, REL_Y)\n");
		ret = -EINVAL;
		goto ioctl_err;
	}

	if (0 > ioctl(uinput_fd, UI_SET_RELBIT, REL_Z)) {
		LOGE("ioctl(fd, UI_SET_RELBIT, REL_Z)\n");
		ret = -EINVAL;
		goto ioctl_err;
	}

	memset(&uinput_dev, 0, sizeof(uinput_dev));
	strncpy(uinput_dev.name, "Wii_Sensors", UINPUT_MAX_NAME_SIZE);
	uinput_dev.id.bustype = 0x1;
	uinput_dev.id.vendor = 0x1;
	uinput_dev.id.product = 0x1;
	uinput_dev.id.version = 0x1;

	if (0 > write(uinput_fd, &uinput_dev, sizeof(uinput_dev))) {
		LOGE("Unable to write to uinput wii_acc device");
		ret = -EINVAL;
		goto write_err;
	}

	if (ioctl(uinput_fd, UI_DEV_CREATE)) {
		LOGE("Unable to ioctl wii_acc device");
		ret = -EINVAL;
		goto ioctl_err;
	}

	return 0;

ioctl_err:
write_err:
	ioctl(uinput_fd, UI_DEV_DESTROY, &uinput_dev);
	close(uinput_fd);

open_err:
	return ret;
}

void wii_acc_close()
{
	ioctl(uinput_fd, UI_DEV_DESTROY, &uinput_dev);
	close(uinput_fd);
}

static int send_event(int fd, uint16_t type, uint16_t code, int32_t value)
{
	struct input_event event;

	memset(&event, 0, sizeof(event));

	event.type      = type;
	event.code      = code;
	event.value     = value;

	return write(fd, &event, sizeof(event));
}

void wii_handle_accelerometer(uint8_t x, uint8_t y, uint8_t z)
{
	unsigned long long new_time_msec;
	struct timespec tick;

	clock_gettime(CLOCK_MONOTONIC, &tick);
	new_time_msec = tick.tv_sec*1000 + tick.tv_nsec/1000000;

	if (new_time_msec >= (acc_time_msec + acc_rate_msec)) {
		acc_time_msec = new_time_msec;

		send_event(uinput_fd, EV_REL, REL_X, (uint32_t) x);
		send_event(uinput_fd, EV_REL, REL_Y, (uint32_t) y);
		send_event(uinput_fd, EV_REL, REL_Z, (uint32_t) z);
		send_event(uinput_fd, EV_SYN, 0, 0);
	}

}


