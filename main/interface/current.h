#pragma once

#include "adc.h"
#include "opamp.h"

static volatile uint16_t raw_currents[2];
static volatile float current[3];
static volatile uint32_t dma_adc_buf[2];
static volatile float adc_current_offsets[2] = {0.0f, 0.0f,};

void init_adc(void);