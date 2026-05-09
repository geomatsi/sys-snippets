#ifndef WII_INPUT_H
#define WII_INPUT_H

#include <inttypes.h>

int wii_input_init(void);
void wii_handle_buttons(uint16_t btn);

#endif	/* WII_INPUT_H */
