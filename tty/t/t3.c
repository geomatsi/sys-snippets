#include <ncurses.h>
#include <stdlib.h>

#define FIELD_H    18
#define FIELD_W    12

#define SCREEN_H   30
#define SCREEN_W   30

int main(void)
{
	unsigned int x, y;
	int row, col;
	WINDOW *win;

	initscr();

	/* init color pairs */

	if(has_colors() == FALSE) {
		endwin();
		printf("Your terminal does not support color\n");
		exit(1);
	}

	use_default_colors();
	start_color();
	curs_set(0);

	init_pair(1, COLOR_BLACK, COLOR_GREEN);
	init_pair(2, COLOR_BLACK, COLOR_YELLOW);
	init_pair(3, COLOR_BLACK, COLOR_RED);
	init_pair(4, COLOR_BLACK, COLOR_GREEN);
	init_pair(5, COLOR_BLACK, COLOR_MAGENTA);

	refresh();

	getmaxyx(stdscr, row, col);
	win = newwin(SCREEN_H, SCREEN_W, (row - SCREEN_H) / 2,  (col - SCREEN_W) / 2 );

	box(win, 0, 0);

	for (x = 0; x < FIELD_W; x++) {
		for (y = 0; y < FIELD_H; y++) {
			int c = y % 5 + 1;
			wattron(win, COLOR_PAIR(c));
			mvwprintw(win, y + (SCREEN_H - FIELD_H) / 2, x + (SCREEN_W - FIELD_W) / 2, "X");
			wattroff(win, COLOR_PAIR(c));
		}
	}


	mvwprintw(win, SCREEN_H - 4, SCREEN_W / 2 - 6, "SCORE: %06d", 1);

	wrefresh(win);
	getch();

	endwin();
	return 0;
}
