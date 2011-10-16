#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <inttypes.h>

#include <sys/types.h>
#include <sys/ioctl.h>

#include <linux/uinput.h>
#include <linux/input.h>

#define LOGE		printf
#define UINPUT_DEV	"/dev/uinput"

static struct uinput_user_dev uinput_dev;
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

	if (0 > ioctl(uinput_fd, UI_SET_EVBIT, EV_KEY)) {
		perror("ioctl UI_SET_EVBIT");
		exit(-1);
	}

	if (0 > ioctl(uinput_fd, UI_SET_EVBIT, EV_REL)) {
		perror("ioctl UI_SET_EVBIT");
		exit(-1);
	}

	if (ioctl(uinput_fd, UI_SET_RELBIT, REL_X) < 0) {
		perror("UI_SET_ABSBIT");
		return -1;
	}

	if (ioctl(uinput_fd, UI_SET_RELBIT, REL_Y) < 0) {
		perror("UI_SET_ABSBIT");
		return -1;
	}

	if (ioctl(uinput_fd, UI_SET_RELBIT, REL_Z) < 0) {
		perror("UI_SET_ABSBIT");
		return -1;
	}

	memset(&uinput_dev, 0, sizeof(uinput_dev));
	strncpy(uinput_dev.name, "uinput_test", UINPUT_MAX_NAME_SIZE);
	uinput_dev.id.bustype = BUS_VIRTUAL;
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

	write(fd, &event, sizeof(struct input_event));
}

void wii_handle_accelerometer(uint8_t x, uint8_t y, uint8_t z)
{

	LOGE("Acc message [%d, %d, %d]\n", x, y, z);

	send_event(uinput_fd, EV_REL, REL_X, (uint32_t) x);
	send_event(uinput_fd, EV_REL, REL_Y, (uint32_t) y);
	send_event(uinput_fd, EV_REL, REL_Z, (uint32_t) z);
	send_event(uinput_fd, EV_SYN, 0, 0);
}

int main(void)
{
	int rc;
	int i  = 0;

	/* Init input subsystem */
	if (0 > (rc = wii_acc_init())) {
		LOGE("Couldn't setup wii_input\n");
		exit(-1);
	}

	while (i++ < 10) {
		wii_handle_accelerometer(120, 120, 150);
		sleep(1);
	}

	wii_acc_close();
}
