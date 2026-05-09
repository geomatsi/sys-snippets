#ifndef WII_ACC_H
#define WII_ACC_H

#include <inttypes.h>

int wii_acc_init(void);
void wii_handle_accelerometer(uint8_t x, uint8_t y, uint8_t z);

#endif	/* WII_ACC_H */
