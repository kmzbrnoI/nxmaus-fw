#ifndef _BUTTONS_H_
#define _BUTTONS_H_

#include <stdbool.h>

#include "hardware.h"

extern uint8_t btn_pressed[BUTTONS_COUNT];
extern void (*on_pressed)(uint8_t button);
extern void (*on_depressed)(uint8_t button);

void btn_update();

#endif
