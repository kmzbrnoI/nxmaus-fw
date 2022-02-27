#ifndef _ENCODER_H_
#define _ENCODER_H_

#include <stddef.h>

// Must be called each 100 us (1 ms is not enough to debounce fast rotation)
void encoder_update();

extern void (*encoder_on_change)(int8_t val);

#endif
