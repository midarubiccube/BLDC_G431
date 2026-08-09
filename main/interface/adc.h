#pragma once

#include "adc.h"
#include "opamp.h"

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern OPAMP_HandleTypeDef hopamp1;
extern OPAMP_HandleTypeDef hopamp2;
extern OPAMP_HandleTypeDef hopamp3;


static volatile uint16_t raw_currents[3];
static volatile float current[3];
static volatile uint32_t dma_adc_buf[3];
static float adc_current_offsets[3] = {2200.0f, 2200.0f, 2200.0f};

void init_adc(void);