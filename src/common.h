#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdbool.h>

#define STEPS_14 0x00
#define STEPS_27 0x01
#define STEPS_28 0x02
#define STEPS_128 0x04

typedef struct {
	uint16_t addr;
	bool free;
	uint8_t step_mode;
	bool forward;
	uint8_t steps;
	uint8_t fa;
	uint8_t fb;
} LocoInfo;

extern LocoInfo loco;

#define CS_STATUS_UNKNOWN 0
#define CS_STATUS_OFF 1
#define CS_STATUS_ON 2
#define CS_STATUS_SERVICE 3

extern uint8_t cs_status;

#endif
