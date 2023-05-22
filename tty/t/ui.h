#ifndef __UI_H__
#define __UI_H__

#define FIELD_H    18
#define FIELD_W    12

#define SCREEN_H   80
#define SCREEN_W   30

#define SSIZE	4

#define LEFT    (1 << 0)
#define RIGHT   (1 << 1)
#define ROTATE  (1 << 2)
#define DOWN    (1 << 3)
#define QUIT    (1 << 7)

int ui_init(void);
void ui_draw_screen(char *, unsigned int, unsigned int);
unsigned int ui_get_keys(void);
void ui_deinit(void);


#endif /* __UI_H__ */
