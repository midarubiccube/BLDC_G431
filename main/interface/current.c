#include "current.h"	

#include "FOC/FOC_calc.h"
#include "timer.h"

void disable_irq_nest();
void enable_irq_nest();

FOC_AB adc_currents_ab;
FOC_DQ adc_currents_dq;
int encoder_resolution = 4096;

float encoder_count = 0;
float encoder_sin;
float encoder_cos;
#include "math.h"

void init_adc(void) {
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);

    HAL_OPAMP_SelfCalibrate(&hopamp1);
	HAL_OPAMP_SelfCalibrate(&hopamp2);

    HAL_OPAMP_Start(&hopamp1);
    HAL_OPAMP_Start(&hopamp2);
	
	HAL_ADC_Start(&hadc2);
	HAL_ADCEx_MultiModeStart_DMA(&hadc1, dma_adc_buf, 2);

	HAL_Delay(100);
    for (uint32_t i = 0; i < 100; i++) {
    	disable_irq_nest();
    	adc_current_offsets[0] += raw_currents[0] / 100.0f;
    	adc_current_offsets[1] += raw_currents[1] / 100.0f;
    	enable_irq_nest();
    	HAL_Delay(1);
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	if (hadc == &hadc1) {
		disable_irq_nest();
		uint32_t adc_data = dma_adc_buf[1];
		raw_currents[1] = adc_data & 0xFFFF;
		raw_currents[0] = adc_data >> 16;
		enable_irq_nest();
		
		current[1] = (raw_currents[0] - adc_current_offsets[0]) / 400.0;
		current[2] = (raw_currents[1] - adc_current_offsets[1]) / 400.0;
		current[0] = ((-current[1]) + (-current[2]));

		adc_currents_ab = FOC_UVWtoAB(current[0], current[1], current[2]);
		encoder_count = ((int16_t)(__HAL_TIM_GET_COUNTER(&htim8)))/-4096.0 * 7.0 * 2.0 *M_PI;
		encoder_sin = arm_sin_f32(encoder_count);
		encoder_cos = arm_cos_f32(encoder_count);
		adc_currents_dq = FOC_ABtoDQ(&adc_currents_ab, encoder_sin, encoder_cos);
	}
}