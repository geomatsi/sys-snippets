#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "ui.h"

struct link {
	struct link *next;
	struct link *prev;
	unsigned int x;
	unsigned int y;
	unsigned int c;
};

char symbols[] = " RLDU*#";

struct link *new_link(unsigned int x, unsigned int y, unsigned int c,
		struct link *next, struct link *prev)
{
	struct link *p;

	p = malloc(sizeof(struct link));
	if (p) {
		p->x = x;
		p->y = y;
		p->c = c;

		p->next = next;
		p->prev = prev;
	}

	return p;
}

bool fit(struct link *p, char *p_field, struct link *head)
{
	struct link *c = head;
	(void)p_field;

	if ((p->x <= 0) || (p->x >= (FIELD_W - 1)))
		return false;

	if ((p->y <= 0) || (p->y >= (FIELD_H - 1)))
		return false;

	while (c) {
		if (c->x == p->x && c->y == p->y)
			return false;
		c = c->next;
	}

	return true;
}

bool find_food(struct link *head, char *p_field)
{
	bool grow = false;
	struct link *p;
	unsigned int x;
	unsigned int y;

	if (p_field[head->y * FIELD_W + head->x] == 5) {
		p_field[head->y * FIELD_W + head->x] = 0;
		grow = true;
	}

	while (grow) {
		x = rand() % (FIELD_W - 2) + 1;	
		y = rand() % (FIELD_H - 2) + 1;	

		p = head;
		while (p) {
			if (p->x == x && p->y == y)
				break;
			p = p->next;
		}

		if (!p) {
			p_field[y * FIELD_W + x] = 5;
			break;
		}
	}

	return grow;
}

unsigned int dir_to_color(unsigned int dir)
{
	switch (dir) {
		case RIGHT:
			return 1;
		case LEFT:
			return 2;
		case DOWN:
			return 3;
		case UP:
			return 4;
		default:
			return 5;
	}
}

void snake_to_screen(struct link *head, char *p_screen)
{
	struct link *p = head;

	assert(p_screen);
	assert(head);

	while (p) {
		p_screen[p->y * FIELD_W + p->x] = symbols[p->c];
		p = p->next;
	}
};

void field_to_screen(char *p_field, char *p_screen)
{
	unsigned int x, y;

	assert(p_screen);
	assert(p_field);

	for (x = 0; x < FIELD_W; x++) {
		for (y = 0; y < FIELD_H; y++) {
			p_screen[y * FIELD_W + x] = symbols[(unsigned int)p_field[y * FIELD_W + x]];
		}
	}
}

int main(void)
{
	bool game_over = false;
	unsigned int x, y, i;
	unsigned int keys;
	struct link *head;
	struct link *tail;
	struct link *p;
	int ret;

	/* field */

	char *p_screen;
	char *p_field;

	/* */

	unsigned int n_direction = LEFT;
	unsigned int n_score = 0;

	/* init ui */

	ret = ui_init();
	if (ret) {
		printf("failed to init user interface backend");
		return ret;
	}

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
			p_field[y * FIELD_W + x] = ((x == 0) || (x == FIELD_W - 1) || (y == 0) || (y == FIELD_H - 1)) ? 6 : 0;
		}
	}

	p_field[10 * FIELD_W + 10] = 5;

	/* allocate and clear screen */

	p_screen = malloc(FIELD_W * FIELD_H * sizeof(char));
	if (!p_screen) {
		perror("failed to allocate screen memory");
		exit(-1);
	}

	for (i = 0; i < FIELD_W * FIELD_H; i++) {
		p_screen[i] = ' ';
	}

	/* allocate and init snake */

	head = tail = new_link(FIELD_W / 2, FIELD_H / 2, dir_to_color(n_direction), NULL, NULL);
	assert(head);

	/* simple main loop */

	while (!game_over) {

		/* timing */

		usleep(200000);

		/* user input */

		keys = ui_get_keys();

		/* logic */

		if (keys & QUIT) {
			game_over = true;
		} else if (keys & LEFT) {
			n_direction = LEFT;
		} else if (keys & RIGHT) {
			n_direction = RIGHT;
		} else if (keys & DOWN) {
			n_direction = DOWN;
		} else if (keys & UP) {
			n_direction = UP;
		} else {
			/* same direction: do nothing */
		}

		p = new_link(head->x, head->y, dir_to_color(n_direction), head, NULL);
		assert(p);
		
		switch (n_direction) {
			case RIGHT:
				p->x = p->x + 1;
				break;;
			case LEFT:
				p->x = p->x - 1;
				break;
			case DOWN:
				p->y = p->y + 1;
				break;
			case UP:
				p->y = p->y - 1;
				break;
			default:
				assert(false);
				break;
		}
		
		if (!fit(p, p_field, head)) {
			game_over = true;
		} else {
			head->prev = p;
			head = p;	
		}

		if (!find_food(head, p_field)) {
			tail = tail->prev;
			free(tail->next);
			tail->next = NULL;
		} else {
			n_score += 10;
		}

		/* dump field to the screen */

		field_to_screen(p_field, p_screen);

		/* dump snake the screen */

		snake_to_screen(head, p_screen);

		/* render screen */

		ui_draw_screen(p_screen, FIELD_W, FIELD_H, n_score);
	}

	ui_deinit();

	printf("game over, your score: %u\n", n_score);

	return 0;
}
