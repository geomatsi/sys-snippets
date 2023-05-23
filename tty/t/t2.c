#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

int main(void)
{
	unsigned int count = 0;
	int running = true;
	int ch;

	initscr();
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	noecho();

	printw("Press ESC to exit.\n");

	while (running) {
		ch = getch();
		count++;

		if (ch == ~0)
			continue;

		if (ch == 'q')
			break;

		if (ch == 0x1b)
			break;

		switch (ch) {
			case KEY_LEFT:
				printw("KEY_LEFT\n");
				break;
			case KEY_RIGHT:
				printw("KEY_RIGHT\n");
				break;
			case KEY_UP:
				printw("KEY_UP\n");
				break;
			case KEY_DOWN:
				printw("KEY_DOWN\n");
				break;
			case 'q':
				running = false;
				break;
			default:
				printw("UNKNOWN: 0x%x\n", ch);
				break;
		}
	}

	endwin();

	printf("count: %u\n", count);
	return 0;
}
