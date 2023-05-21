#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "ui.h"

#define SHAPES    7

char shape[SHAPES][SSIZE * SSIZE] = {
	{
		'.', '.', 'X', '.',
		'.', '.', 'X', '.',
		'.', '.', 'X', '.',
		'.', '.', 'X', '.',
	},
	{
		'.', '.', '.', '.',
		'.', 'X', 'X', '.',
		'.', 'X', 'X', '.',
		'.', '.', '.', '.',
	},
	{
		'.', '.', 'X', '.',
		'.', 'X', 'X', '.',
		'.', 'X', '.', '.',
		'.', '.', '.', '.',
	},
	{
		'.', 'X', '.', '.',
		'.', 'X', 'X', '.',
		'.', '.', 'X', '.',
		'.', '.', '.', '.',
	},
	{
		'.', '.', 'X', '.',
		'.', 'X', 'X', '.',
		'.', '.', 'X', '.',
		'.', '.', '.', '.',
	},
	{
		'.', '.', '.', '.',
		'.', 'X', 'X', '.',
		'.', '.', 'X', '.',
		'.', '.', 'X', '.',
	},
	{
		'.', '.', '.', '.',
		'.', 'X', 'X', '.',
		'.', 'X', '.', '.',
		'.', 'X', '.', '.',
	},
};

char symbols[] = " ABCDEFG=#";

int rotate(int x, int y, int r)
{
	assert(x < SSIZE);
	assert(y < SSIZE);

	switch (r % 4) {
		case 0: return y * SSIZE + x;
		case 1: return 12 + y - (x * SSIZE);
		case 2: return 15 - (y * SSIZE) - x;
		case 3: return 3 - y + (x * SSIZE);
	}

	return 0;
}

bool fit(char *p_field, int sn, int r, int px, int py)
{
	unsigned int pi, fi;
	unsigned int x, y;

	assert(sn < SHAPES);
	assert(p_field);

	for (x = 0; x < SSIZE; x++) {
		for (y = 0; y < SSIZE; y++) {
			fi = (py + y) * FIELD_W + (px + x);	
			pi = rotate(x, y, r);

			if ((px + x) < FIELD_W && (py + y) < FIELD_H) {
				if (shape[sn][pi] == 'X' && p_field[fi] != 0)
					return false;
			}
		}
	}

	return true;
}

void field_to_screen(char *p_field, char *p_screen)
{
	unsigned int x, y;

	assert(p_screen);
	assert(p_field);

	for (x = 0; x < FIELD_W; x++) {
		for (y = 0; y < FIELD_H; y++) {
			p_screen[(y + 2) * SCREEN_W + (x + 2)] = symbols[(unsigned int)p_field[y * FIELD_W + x]];
		}
	}
}

int main(void)
{
	bool game_over = false;
	unsigned int i, x, y;
	unsigned int keys;

	/* field */

	char *p_screen;
	char *p_field;

	unsigned int full_rows[4] = { ~0U };
	bool remove_full_row = false;

	/* current shape: shape num, rotation, position */

	unsigned int cs = 0;
	unsigned int cr = 0;
	unsigned int cx = FIELD_W / 2;
	unsigned int cy = 0;

	/* timing and scoring */

	unsigned int n_pieces = 1;
	unsigned int n_speed = 20;
	unsigned int n_count = 0;
	unsigned int n_score = 0;
	bool n_down = false;

	/* init random */

	srand(arc4random());

	/* allocate and init field */

	p_field = malloc(FIELD_W * FIELD_H * sizeof(char));
	if (!p_field) {
		perror("failed to allocate field memory");
		exit(-1);
	}

	for (x = 0; x < FIELD_W; x++) {
		for (y = 0; y < FIELD_H; y++) {
			p_field[y * FIELD_W + x] = ((x == 0) || (x == FIELD_W - 1) || (y == FIELD_H - 1)) ? 9 : 0;
		}
	}

	/* allocate and clear screen */

	p_screen = malloc(SCREEN_W * SCREEN_H * sizeof(char));
	if (!p_screen) {
		perror("failed to allocate screen memory");
		exit(-1);
	}

	for (i = 0; i < SCREEN_W * SCREEN_H; i++) {
		p_screen[i] = ' ';
	}

	/* simple main loop */

	while (!game_over) {

		/* timing */

		usleep(50000);

		if (++n_count >= n_speed)
			n_down = true;

		/* user input */

		keys = get_keys();

		/* logic */

		if (keys & LEFT) {
			if (fit(p_field, cs, cr, cx - 1, cy))
				cx = cx - 1;
		}

		if (keys & RIGHT) {
			if (fit(p_field,cs, cr, cx + 1, cy))
				cx = cx + 1;
		}
		
		if (keys & DOWN) {
			if (fit(p_field,cs, cr, cx, cy + 1))
				cy = cy + 1;
		}

		if (keys & ROTATE) {
			if (fit(p_field,cs, cr + 1, cx, cy))
				cr = cr + 1;
		}

		if (keys & QUIT) {
			game_over = true;
		}

		if (n_down) {
			if (fit(p_field,cs, cr, cx, cy + 1)) {
				cy = cy + 1;
			} else {
				/* increase speed after every 5 pieces */

				if (n_pieces++ % 10 == 0)
					n_speed = n_speed > 1 ? n_speed - 1 : n_speed;

				/* lock shape in the field */

				for (x = 0; x < SSIZE; x++) {
					for (y = 0; y < SSIZE; y++) {
						if (shape[cs][rotate(x, y, cr)] == 'X') {
							p_field[(cy + y) * FIELD_W + (cx + x)] = cs + 1;
						}
					}
				}

				/* check full rows */

				for (y = 0; y < SSIZE; y++) {

					if ((y + cy) < (FIELD_H - 1)) {
						bool full_row_ready = true;

						for (x = 0; x < FIELD_W; x++) {
							if (p_field[(cy + y) * FIELD_W + x] == 0) {
								full_row_ready = false;
								break;
							}
						}

						if (full_row_ready) {
							for (x = 0; x < FIELD_W; x++) {
								p_field[(cy + y) * FIELD_W + x] = 8;
							}

							full_rows[y] = cy + y;
							remove_full_row = true;
						}
					}
				}

				/* select the next piece */

				cs = rand() % SHAPES;
				cr = 0;
				cx = FIELD_W / 2;
				cy = 0;

				/* check if there is a room for the next piece */
				if (!fit(p_field,cs, cr, cx, cy))
					game_over = true;

			}

			n_down = false;
			n_count = 0;
		}

		/* dump field to the screen */

		field_to_screen(p_field, p_screen);

		/* remove full rows and 'animate' process */

		if (remove_full_row) {
			draw_screen(p_screen, SCREEN_W, SCREEN_H);

			for (i = 0; i < sizeof(full_rows) / sizeof(full_rows[0]); i++) {
				unsigned int row = full_rows[i];

				if (row == ~0U)
					continue;
				memmove(p_field + FIELD_W, p_field, FIELD_W * row);

				/* reward  for full row */
				n_score += 25;
			}

			/* again dump field to the screen */

			field_to_screen(p_field, p_screen);

			memset(full_rows, ~0, sizeof(full_rows));
			remove_full_row = false;
		}

		/* dump current piece to the screen */

		for (x = 0; x < SSIZE; x++) {
			for (y = 0; y < SSIZE; y++) {
				if (shape[cs][rotate(x, y, cr)] == 'X')
					p_screen[(cy + y + 2) * SCREEN_W + (cx + x + 2)] = symbols[cs + 1];
			}
		}

		/* render screen */

		draw_screen(p_screen, SCREEN_W, SCREEN_H);
	}

	printf("game over, your score: %u\n", n_score);

	return 0;
}
