#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/ioctl.h>

#include <linux/uinput.h>
#include <linux/input.h>

#include <wii_input.h>

#if ANDROID

#define LOG_TAG "wiid_input"
#define LOG_NDEBUG 0

#include <cutils/log.h>

#else

#include <linux/input.h>
#include <wii_log.h>

#endif /* ANDROID */

#define UINPUT_DEV	"/dev/uinput"

static struct uinput_user_dev uinput_dev;
static int uinput_fd = -1;

/* */

static int uinput_keypad_init(int fd, struct uinput_user_dev *uinp)
{
	int i;

	memset(uinp, 0, sizeof(*uinp));
	strncpy(uinp->name, "Wii_Keypad", UINPUT_MAX_NAME_SIZE);
	uinp->id.bustype = BUS_HOST;
	uinp->id.version = 1;

	ioctl(fd, UI_SET_EVBIT, EV_KEY);
	ioctl(fd, UI_SET_EVBIT, EV_SYN);
	/* ioctl(fd, UI_SET_EVBIT, EV_REP); */
	for (i = 0; i < KEY_MAX; i++)
	      ioctl(fd, UI_SET_KEYBIT, i);

	write(fd, uinp, sizeof(*uinp));

	if (ioctl(fd, UI_DEV_CREATE)) {
		LOGE("Unable to create uinput wii device");
		return -1;
	}

	return 0;
}

static int uinput_write(int uinput_fd,
                         uint16_t type, uint16_t code, int32_t value)
{
	struct input_event event;

	memset(&event, 0, sizeof(event));
	gettimeofday(&event.time, 0); /* This should not be able to fail ever */
	event.type = type;
	event.code = code;
	event.value = value;
	if (write(uinput_fd, &event, sizeof(event)) != sizeof(event))
		return -1;
	return 0;
}

static int uinput_write_syn(int uinput_fd,
			     uint16_t type, uint16_t code, int32_t value)
{
	if (uinput_write(uinput_fd, type, code, value))
		return -1;
	return uinput_write(uinput_fd, EV_SYN, SYN_REPORT, 0);
}

int uinput_press(int uinput_fd, uint16_t code)
{
	return uinput_write_syn(uinput_fd, EV_KEY, code, 1);
}

int uinput_release(int uinput_fd, uint16_t code)
{
	return uinput_write_syn(uinput_fd, EV_KEY, code, 0);
}

int uinput_click(int uinput_fd, uint16_t code)
{
	if (uinput_press(uinput_fd, code))
		return -1;
	return uinput_release(uinput_fd, code);
}

int wii_input_init()
{
	int ret = 0;

	uinput_fd = open(UINPUT_DEV, O_RDWR);
	if (uinput_fd < 0) {
		LOGE("Error opening %s\n", UINPUT_DEV);
		ret = -EINVAL;
		return ret;
	}

	/* init uinput device */
	if (uinput_keypad_init(uinput_fd, &uinput_dev) < 0) {
		LOGE("Error keypad init%s\n", UINPUT_DEV);
		ret = -EINVAL;
		ioctl(uinput_fd, UI_DEV_DESTROY, &uinput_dev);
		close(uinput_fd);
		return ret;
	}

	return ret;
}

void wii_input_close()
{
	ioctl(uinput_fd, UI_DEV_DESTROY, &uinput_dev);
	close(uinput_fd);
}

void wii_handle_buttons(uint16_t btn)
{
	char key;

	/* FIXME: simple mapping */

#if ANDROID

	/*
	 * This is just scan code to scan code translation:
	 * we can't exceed android scan code length.
	 * For actual mapping see: device/ti/beagleboard/Wii_Keypad.kl
	 */

	switch(btn) {
	case 0x0010:
		key = 10;
		break;
	case 0x1000:
		key = 11;
		break;
	case 0x0080:
		key = 12;
		break;
	case 0x0008:
		key = 13;
		break;
	case 0x0004:
		key = 14;
		break;
	case 0x0200:
		key = 15;
		break;
	case 0x0100:
		key = 16;
		break;
	case 0x0400:
		key = 17;
		break;
	case 0x0800:
		key = 18;
		break;
	default:
		key = 0;
		break;

	}

#else	/* LINUX */

	/* In Linux we inject key events according to /usr/include/linux/input.h */

	switch(btn) {
	case 0x0010:
		key = KEY_9;
		break;
	case 0x1000:
		key = KEY_0;
		break;
	case 0x0080:
		key = KEY_HOME;
		break;
	case 0x0008:
		key = KEY_BACKSPACE;
		break;
	case 0x0004:
		key = KEY_ENTER;
		break;
	case 0x0200:
		key = KEY_RIGHT;
		break;
	case 0x0100:
		key = KEY_LEFT;
		break;
	case 0x0400:
		key = KEY_DOWN;
		break;
	case 0x0800:
		key = KEY_UP;
		break;
	default:
		key = 0;
		break;

	}

#endif	/* ANDROID */

	LOGD("press key %d\n", key);
	uinput_click(uinput_fd, key);
}
