#include <ncurses.h>

#include "ui.h"

int ui_init(void)
{
	initscr();
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	noecho();
	raw();

	/* init color pairs */

	if(has_colors() == TRUE) {
		start_color();
		init_pair(1, COLOR_BLACK, COLOR_CYAN);
		init_pair(2, COLOR_BLACK, COLOR_YELLOW);
		init_pair(3, COLOR_BLACK, COLOR_RED);
		init_pair(4, COLOR_BLACK, COLOR_GREEN);
		init_pair(5, COLOR_BLACK, COLOR_MAGENTA);
		init_pair(6, COLOR_BLACK, COLOR_RED);    // TODO: need orange
		init_pair(7, COLOR_BLACK, COLOR_BLUE);
		init_pair(8, COLOR_BLACK, COLOR_BLACK);  // TODO: need gray
		init_pair(9, COLOR_WHITE, COLOR_WHITE);
	}

	return 0;
}

void ui_draw_screen(char *p_screen, unsigned int width, unsigned int height)
{
	unsigned int x, y, c;

	for (x = 0; x < width; x++) {
		for (y = 0; y < height; y++) {
			char ch = p_screen[y * width + x];

			switch (ch) {
				case 'A':
					c = 1;
					break;
				case 'B':
					c = 2;
					break;
				case 'C':
					c = 3;
					break;
				case 'D':
					c = 4;
					break;
				case 'E':
					c = 5;
					break;
				case 'F':
					c = 6;
					break;
				case 'G':
					c = 7;
					break;
				case '=':
				case '#':
					c = 8;
					break;
				default:
					c = 9;
					break;
			}

			if(has_colors() == TRUE)
				attron(COLOR_PAIR(c));

			mvprintw(y, x, "%c", ch);
		}
	}

	refresh();
}

unsigned int ui_get_keys(void)
{
	int cmds = 0;
	int ch;

	ch = getch();
	switch (ch) {
		case KEY_LEFT:
			cmds |= LEFT;
			break;
		case KEY_RIGHT:
			cmds |= RIGHT;
			break;
		case KEY_UP:
			cmds |= ROTATE;
			break;
		case KEY_DOWN:
			cmds |= DOWN;
			break;
		case 0x1b:
		case 'q':
			cmds |= QUIT;
		default:
			break;
	}

	return cmds;
}

void ui_deinit(void)
{
	endwin();
}
