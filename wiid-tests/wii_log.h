#ifndef WII_LOG_H
#define WII_LOG_H

#include <stdio.h>

#define LOG(level, ...) \
	        ((void)printf(level ": " __VA_ARGS__))

#define LOGV(...)   LOG("V", __VA_ARGS__)
#define LOGD(...)   LOG("D", __VA_ARGS__)
#define LOGI(...)   LOG("I", __VA_ARGS__)
#define LOGW(...)   LOG("W", __VA_ARGS__)
#define LOGE(...)   LOG("E", __VA_ARGS__)

#endif	/* WII_LOG_H */
