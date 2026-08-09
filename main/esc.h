#pragma once

#include "FOC/FOC_calc.h"

#define PWM_PERIOD_COUNTS 4250.0f

extern FOC_AB adc_currents_ab;
extern FOC_DQ adc_currents_dq;

void MotorControlTask();
void motor_controller_setup();
void khz_task();