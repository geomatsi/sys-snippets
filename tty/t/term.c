#include <sys/types.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#include "ui.h"

#define CLEAR    "\e[2J"
#define POSYX    "\e[%d;%dH"
#define COLOR    "\e[48;5;%dm"
#define RESET    "\e[0m"

#define EMPTY    7
#define CYAN     81
#define YELLOW   227
#define PURPLE   165
#define BLUE     21
#define ORANGE   215
#define GREEN    47
#define RED      197
#define GRAY     245

int ui_init(void)
{
	printf(CLEAR);
	return 0;
}

void ui_draw_screen(char *p_screen, unsigned int width, unsigned int height, int score)
{
	unsigned int x, y;

	for (x = 0; x < width; x++) {
		for (y = 0; y < height; y++) {
			char ch = p_screen[y * width + x];
			int color;

			switch (ch) {
				case 'A':
					color = CYAN;
					break;
				case 'B':
					color = YELLOW;
					break;
				case 'C':
					color = RED;
					break;
				case 'D':
					color = GREEN;
					break;
				case 'E':
					color = PURPLE;
					break;
				case 'F':
					color = ORANGE;
					break;
				case 'G':
					color = BLUE;
					break;
				case '=':
				case '#':
					color = GRAY;
					break;
				default:
					color = EMPTY;
					break;
			}

			printf(POSYX COLOR "%c", y + (SCREEN_H - height) / 2, x + (SCREEN_W - width) / 2, color, ch);
			fflush(stdout);
		}
	}

	printf(POSYX "SCORE: %06d", SCREEN_H - 4, SCREEN_W / 2 - 6, score);
	printf(POSYX, 200, 200);
	fflush(stdout);
}

unsigned int ui_get_keys(void)
{
	struct termios tp1, tp2;
	char buffer[16] = {0};
	int c, fc;
	int cmds = 0;

	tcgetattr(0, &tp1);
	tcgetattr(0, &tp2);
	fc = fcntl(0, F_GETFL);

	tp2.c_iflag &= ~ICRNL;
	tp2.c_lflag &= ~ICANON;
	tp2.c_lflag &= ~ECHO;
	tp2.c_cc[VMIN ] = 1;
	tp2.c_cc[VTIME] = 0;
	tp2.c_cc[VINTR] = 0xFF;
	tp2.c_cc[VSUSP] = 0xFF;
	tp2.c_cc[VQUIT] = 0xFF;

	fcntl(0, F_SETFL, fc | O_NONBLOCK);
	tcsetattr(0, TCSANOW, &tp2);

	c = read(0, buffer, sizeof(buffer));
	switch (c) {
		case 1:
			switch (buffer[0]) {
				case 0x1b: /* ESC */
					cmds |= QUIT;
					break;
				default:
					break;
			}
			break;
		case 3: /* arrow keys */
			if (!strncmp(buffer, "\x1b\x5b\x44", 3))
				cmds |= LEFT;
			if (!strncmp(buffer, "\x1b\x5b\x43", 3))
				cmds |= RIGHT;
			if (!strncmp(buffer, "\x1b\x5b\x41", 3))
				cmds |= ROTATE;
			if (!strncmp(buffer, "\x1b\x5b\x42", 3))
				cmds |= DOWN;
			break;
		default:
			cmds = 0;
			break;
	}

	tcsetattr(0,TCSANOW,&tp1);
	fcntl(0, F_SETFL, fc);
	fflush(stdin);

	return cmds;
}

void ui_deinit(void)
{

}
