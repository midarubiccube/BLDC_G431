#pragma once

#include "tim.h"

#ifdef __cplusplus
extern "C" {
#endif

void init_timer(void);
void timer_setDuty(float u, float v, float w);
void set_encoder_offset();

void set_control_task(void (*task)());
void set_khz_task(void (*task)());

#ifdef __cplusplus
}
#endif