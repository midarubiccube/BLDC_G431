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

#ifdef __cplusplus
}
#endif
static uint32_t irq_cnt = 0;

static volatile uint8_t adc2_rank_index = 0;