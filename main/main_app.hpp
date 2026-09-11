#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void main_setup(void);
void main_loop(void);

void khz_task();

void disable_irq_nest();
void enable_irq_nest();

#pragma pack(push, 1)
struct BLDCPacket {
    uint8_t mode;
    int16_t rps_target;          
    float  angle_target;
    uint16_t rps;          
};
#pragma pack(pop)

#ifdef __cplusplus
}
#endif
static uint32_t irq_cnt = 0;

static volatile uint8_t adc2_rank_index = 0;