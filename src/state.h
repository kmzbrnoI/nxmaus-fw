#ifndef _STATE_H_
#define _STATE_H_

#include <stdint.h>

#define ST_XN_UNADDRESSED 0
#define ST_CS_STATUS_ASKING 1
#define ST_LOCO_STATUS_AKSKING 2
#define ST_LOCO_MINE 3
#define ST_LOCO_STOLEN 4
#define ST_LOCO_RELEASED 5

extern uint8_t state;

void state_show(uint16_t counter);

#endif
