#pragma once

#include "tim.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern int16_t encoder_offset;

void init_timer(void);
void timer_setDuty(float u, float v, float w);

void set_control_task(void (*task)());
void set_khz_task(void (*task)());