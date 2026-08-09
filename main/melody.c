/*
 * melody_controller.c
 *
 *  Created on: Aug 29, 2025
 *      Author: Doraemonjayo
 */

#include "main_app.h"

#include "melody_defines.h"
#include "FOC/FOC_calc.h"
#include "interface/timer.h"

float melody_freq = 1000.0f;
float melody_volume = 0.0f;

FOC_DQ dq_out = {0};
FOC_AB ab_out = {0};
FOC_UVW uvw_out = {0};
FOC_DQ dq_current = {0};
FOC_AB ab_current = {0};
FOC_UVW uvw_current = {0};

extern float encoder_sin;
extern float encoder_cos;

void melodyControlTask() {
	static uint32_t tick = 0;

	if (melody_freq < 1.0f) melody_freq = 1.0f;
	if (melody_freq > 2000.0f) melody_freq = 2000.0f;
	if (melody_volume > 1.0f) melody_volume = 1.0f;
	if (melody_volume < 0.0f) melody_volume = 0.0f;

	float period = 20000.0f / melody_freq;
	uint32_t cnt = tick - (uint32_t)((float)(uint32_t)((float)tick / period) * period);
	if (cnt == 0) {
		dq_out.d = melody_volume;
	} else if (cnt == (uint32_t)(period / 2.0f)) {
		dq_out.d = -melody_volume;
	} else {
		dq_out.d = 0.0f;
	}
	dq_out.q = 0.0f;

	float norm;
	arm_sqrt_f32(dq_out.d * dq_out.d + dq_out.q * dq_out.q, &norm);
	if (norm > 1.0f) {
		dq_out.d /= norm;
		dq_out.q /= norm;
	}

	FOC_DQtoAB(&dq_out, encoder_sin, encoder_cos, &ab_out);
	FOC_ABtoUVW(&ab_out, &uvw_out);
	timer_setDuty(uvw_out.u, uvw_out.v, uvw_out.w);

	tick++;
}

