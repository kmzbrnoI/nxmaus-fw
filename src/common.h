#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdbool.h>
#include <stdint.h>

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

static inline uint8_t loco_addr_hi() { return loco.addr >> 8; }
static inline uint8_t loco_addr_lo() { return loco.addr & 0xFF; }

void loco_send_seedir();
void loco_send_fa();
void loco_send_fb_58();
void loco_send_fb_912();

#endif
