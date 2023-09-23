#ifndef __UI_H__
#define __UI_H__

#define FIELD_H    25
#define FIELD_W    95

#define SCREEN_H   30
#define SCREEN_W   100

#define SSIZE	4

#define LEFT    (1 << 0)
#define RIGHT   (1 << 1)
#define UP      (1 << 2)
#define DOWN    (1 << 3)
#define QUIT    (1 << 7)

int ui_init(void);
void ui_draw_screen(char *, unsigned int, unsigned int, int);
unsigned int ui_get_keys(void);
void ui_deinit(void);


#endif /* __UI_H__ */
