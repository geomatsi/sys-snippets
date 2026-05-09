#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <bluetooth/bluetooth.h>
#include <cwiid.h>

#ifdef ANDROID

#define LOG_TAG "wiid"
#define LOG_NDEBUG 0
#include <cutils/log.h>
#include <android/log.h>

#else

#include <wii_log.h>

#endif

#include <wii_input.h>
#include <wii_acc.h>

cwiid_mesg_callback_t cwiid_callback;

#define CONNECT_TIMEOUT 2

#define toggle_bit(bf,b)	(bf) = ((bf) & b) ? ((bf) & ~(b)) : ((bf) | (b))
#define set_bit(bf, b) 		(bf) = ((bf) | (b))

int set_led_state(cwiid_wiimote_t *wiimote, unsigned char led_state);
void set_rpt_mode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode);
void print_state(struct cwiid_state *state);

cwiid_err_t err;

void err(cwiid_wiimote_t *wiimote, const char *s, va_list ap)
{

#ifdef ANDROID
	if (wiimote)
	        __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "%d:", cwiid_get_id(wiimote));
	else
	        __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "-1:");
		printf("-1:");

	__android_log_vprint(ANDROID_LOG_ERROR, LOG_TAG, s, ap);
	__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "\n");
#else
	vfprintf(stderr, s, ap);
#endif /* ANDROID */
}

int main(int argc, char *argv[])
{
	cwiid_wiimote_t *wiimote;	/* wiimote handle */
	struct cwiid_state state;	/* wiimote state */
	bdaddr_t bdaddr;	/* bluetooth device address */
	int rc;

	unsigned char mesg = 0;
	unsigned char led_state = 0;
	unsigned char rpt_mode = 0;
	unsigned char rumble = 0;

	cwiid_set_err(err);

	/* Init input subsystem */
	if (0 > (rc = wii_input_init())) {
		LOGE("Couldn't setup wii_input\n");
		exit(-1);
	}

	/* Init input subsystem */
	if (0 > (rc = wii_acc_init())) {
		LOGE("Couldn't setup wii_acc\n");
		exit(-1);
	}

start:
	/* Connect to address given on command-line, if present */

	if (argc > 1) {
		str2ba(argv[1], &bdaddr);
	}
	else {
		bdaddr = *BDADDR_ANY;
	}


	/* Connect to the wiimote */
	while (1) {
		LOGV("Put wiimote in discoverable mode now (press 1+2)...\n");
		wiimote = cwiid_open(&bdaddr, 0);
		if (wiimote) {
			break;
		} else {
			LOGV("Unable to connect to wiimote, wait %d sec...\n", CONNECT_TIMEOUT);
			sleep(CONNECT_TIMEOUT);
		}
	}

	if (cwiid_set_mesg_callback(wiimote, cwiid_callback)) {
		LOGE("Unable to set message callback\n");
	}

	/* request messages from WiiRemote */
	if (cwiid_enable(wiimote, CWIID_FLAG_MESG_IFC)) {
		LOGE("Error enabling messages\n");
	}

	/* toggle buttons info */
	set_bit(rpt_mode, CWIID_RPT_BTN);
	set_rpt_mode(wiimote, rpt_mode);

	/* toggle acc info */
	set_bit(rpt_mode, CWIID_RPT_ACC);
	set_rpt_mode(wiimote, rpt_mode);

	do {
		toggle_bit(led_state, CWIID_LED1_ON);
		if (set_led_state(wiimote, led_state))
			break;
		sleep(1);

		toggle_bit(led_state, CWIID_LED2_ON);
		if (set_led_state(wiimote, led_state))
			break;
		sleep(1);

		toggle_bit(led_state, CWIID_LED3_ON);
		if (set_led_state(wiimote, led_state))
			break;
		sleep(1);

		toggle_bit(led_state, CWIID_LED4_ON);
		if (set_led_state(wiimote, led_state))
			break;
		sleep(1);

	} while (1);

	if (cwiid_close(wiimote)) {
		LOGE("Error on wiimote disconnect\n");
	}

#ifdef ANDROID
	goto start;
#endif

	return 0;
}

int set_led_state(cwiid_wiimote_t *wiimote, unsigned char led_state)
{
	if (cwiid_set_led(wiimote, led_state)) {
		LOGE("Error setting LEDs \n");
		return -1;
	}

	return 0;
}

void set_rpt_mode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode)
{
	if (cwiid_set_rpt_mode(wiimote, rpt_mode)) {
		LOGE("Error setting report mode\n");
	}
}

void print_state(struct cwiid_state *state)
{
	int i;
	int valid_source = 0;

	LOGV("Report Mode:");
	if (state->rpt_mode & CWIID_RPT_STATUS) LOGV(" STATUS");
	if (state->rpt_mode & CWIID_RPT_BTN) LOGV(" BTN");
	if (state->rpt_mode & CWIID_RPT_ACC) LOGV(" ACC");
	if (state->rpt_mode & CWIID_RPT_IR) LOGV(" IR");
	if (state->rpt_mode & CWIID_RPT_NUNCHUK) LOGV(" NUNCHUK");
	if (state->rpt_mode & CWIID_RPT_CLASSIC) LOGV(" CLASSIC");
	if (state->rpt_mode & CWIID_RPT_BALANCE) LOGV(" BALANCE");
	if (state->rpt_mode & CWIID_RPT_MOTIONPLUS) LOGV(" MOTIONPLUS");
	LOGV("\n");

	LOGV("Active LEDs:");
	if (state->led & CWIID_LED1_ON) LOGV(" 1");
	if (state->led & CWIID_LED2_ON) LOGV(" 2");
	if (state->led & CWIID_LED3_ON) LOGV(" 3");
	if (state->led & CWIID_LED4_ON) LOGV(" 4");
	LOGV("\n");

	LOGV("Rumble: %s\n", state->rumble ? "On" : "Off");

	LOGV("Battery: %d%%\n",
	       (int)(100.0 * state->battery / CWIID_BATTERY_MAX));

	LOGV("Buttons: %X\n", state->buttons);

	LOGV("Acc: x=%d y=%d z=%d\n", state->acc[CWIID_X], state->acc[CWIID_Y],
	       state->acc[CWIID_Z]);

	LOGV("IR: ");
	for (i = 0; i < CWIID_IR_SRC_COUNT; i++) {
		if (state->ir_src[i].valid) {
			valid_source = 1;
			LOGV("(%d,%d) ", state->ir_src[i].pos[CWIID_X],
			                   state->ir_src[i].pos[CWIID_Y]);
		}
	}
	if (!valid_source) {
		LOGV("no sources detected");
	}
	LOGV("\n");

	switch (state->ext_type) {
	case CWIID_EXT_NONE:
		LOGV("No extension\n");
		break;
	case CWIID_EXT_UNKNOWN:
		LOGV("Unknown extension attached\n");
		break;
	case CWIID_EXT_NUNCHUK:
		LOGV("Nunchuk: btns=%.2X stick=(%d,%d) acc.x=%d acc.y=%d "
		       "acc.z=%d\n", state->ext.nunchuk.buttons,
		       state->ext.nunchuk.stick[CWIID_X],
		       state->ext.nunchuk.stick[CWIID_Y],
		       state->ext.nunchuk.acc[CWIID_X],
		       state->ext.nunchuk.acc[CWIID_Y],
		       state->ext.nunchuk.acc[CWIID_Z]);
		break;
	case CWIID_EXT_CLASSIC:
		LOGV("Classic: btns=%.4X l_stick=(%d,%d) r_stick=(%d,%d) "
		       "l=%d r=%d\n", state->ext.classic.buttons,
		       state->ext.classic.l_stick[CWIID_X],
		       state->ext.classic.l_stick[CWIID_Y],
		       state->ext.classic.r_stick[CWIID_X],
		       state->ext.classic.r_stick[CWIID_Y],
		       state->ext.classic.l, state->ext.classic.r);
		break;
	case CWIID_EXT_BALANCE:
		LOGV("Balance: right_top=%d right_bottom=%d "
		       "left_top=%d left_bottom=%d\n",
		       state->ext.balance.right_top,
		       state->ext.balance.right_bottom,
		       state->ext.balance.left_top,
		       state->ext.balance.left_bottom);
		break;
	case CWIID_EXT_MOTIONPLUS:
		LOGV("MotionPlus: angle_rate=(%d,%d,%d) low_speed=(%d,%d,%d)\n",
		       state->ext.motionplus.angle_rate[0],
		       state->ext.motionplus.angle_rate[1],
		       state->ext.motionplus.angle_rate[2],
		       state->ext.motionplus.low_speed[0],
		       state->ext.motionplus.low_speed[1],
		       state->ext.motionplus.low_speed[2]);
		break;
	}
}

void cwiid_callback(cwiid_wiimote_t *wiimote, int mesg_count,
                    union cwiid_mesg mesg[], struct timespec *timestamp)
{
	int i, j;
	int valid_source;

	for (i=0; i < mesg_count; i++)
	{
		switch (mesg[i].type) {

		case CWIID_MESG_BTN:
			wii_handle_buttons(mesg[i].btn_mesg.buttons);
			break;
		case CWIID_MESG_ACC:
			wii_handle_accelerometer(
                		mesg[i].acc_mesg.acc[CWIID_X],
				mesg[i].acc_mesg.acc[CWIID_Y],
				mesg[i].acc_mesg.acc[CWIID_Z]
			);
			break;

		/* further data is currently not used */

		case CWIID_MESG_STATUS:
			LOGV("Status Report: battery=%d extension=",
			       mesg[i].status_mesg.battery);
			switch (mesg[i].status_mesg.ext_type) {
			case CWIID_EXT_NONE:
				LOGV("none");
				break;
			case CWIID_EXT_NUNCHUK:
				LOGV("Nunchuk");
				break;
			case CWIID_EXT_CLASSIC:
				LOGV("Classic Controller");
				break;
			case CWIID_EXT_BALANCE:
				LOGV("Balance Board");
				break;
			case CWIID_EXT_MOTIONPLUS:
				LOGV("MotionPlus");
				break;
			default:
				LOGV("Unknown Extension");
				break;
			}
			LOGV("\n");
			break;
		case CWIID_MESG_IR:
			LOGV("IR Report: ");
			valid_source = 0;
			for (j = 0; j < CWIID_IR_SRC_COUNT; j++) {
				if (mesg[i].ir_mesg.src[j].valid) {
					valid_source = 1;
					LOGV("(%d,%d) ", mesg[i].ir_mesg.src[j].pos[CWIID_X],
					                   mesg[i].ir_mesg.src[j].pos[CWIID_Y]);
				}
			}
			if (!valid_source) {
				LOGV("no sources detected");
			}
			LOGV("\n");
			break;
		case CWIID_MESG_NUNCHUK:
			LOGV("Nunchuk Report: btns=%.2X stick=(%d,%d) acc.x=%d acc.y=%d "
			       "acc.z=%d\n", mesg[i].nunchuk_mesg.buttons,
			       mesg[i].nunchuk_mesg.stick[CWIID_X],
			       mesg[i].nunchuk_mesg.stick[CWIID_Y],
			       mesg[i].nunchuk_mesg.acc[CWIID_X],
			       mesg[i].nunchuk_mesg.acc[CWIID_Y],
			       mesg[i].nunchuk_mesg.acc[CWIID_Z]);
			break;
		case CWIID_MESG_CLASSIC:
			LOGV("Classic Report: btns=%.4X l_stick=(%d,%d) r_stick=(%d,%d) "
			       "l=%d r=%d\n", mesg[i].classic_mesg.buttons,
			       mesg[i].classic_mesg.l_stick[CWIID_X],
			       mesg[i].classic_mesg.l_stick[CWIID_Y],
			       mesg[i].classic_mesg.r_stick[CWIID_X],
			       mesg[i].classic_mesg.r_stick[CWIID_Y],
			       mesg[i].classic_mesg.l, mesg[i].classic_mesg.r);
			break;
		case CWIID_MESG_BALANCE:
			LOGV("Balance Report: right_top=%d right_bottom=%d "
			       "left_top=%d left_bottom=%d\n",
			       mesg[i].balance_mesg.right_top,
			       mesg[i].balance_mesg.right_bottom,
			       mesg[i].balance_mesg.left_top,
			       mesg[i].balance_mesg.left_bottom);
			break;
		case CWIID_MESG_MOTIONPLUS:
			LOGV("MotionPlus Report: angle_rate=(%d,%d,%d) low_speed=(%d,%d,%d)\n",
			       mesg[i].motionplus_mesg.angle_rate[0],
			       mesg[i].motionplus_mesg.angle_rate[1],
			       mesg[i].motionplus_mesg.angle_rate[2],
			       mesg[i].motionplus_mesg.low_speed[0],
			       mesg[i].motionplus_mesg.low_speed[1],
			       mesg[i].motionplus_mesg.low_speed[2]);
			break;
		case CWIID_MESG_ERROR:
			LOGE("Error on wiimote disconnect\n");
			break;
		default:
			LOGE("Unknown Report");
			break;
		}
	}
}
