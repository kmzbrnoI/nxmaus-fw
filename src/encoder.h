#ifndef _ENCODER_H_
#define _ENCODER_H_

#include <stddef.h>

void encoder_update();

extern void (*encoder_on_change)(int8_t val);

#endif
