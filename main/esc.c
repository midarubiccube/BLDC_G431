#include "esc.h"

#include "FOC/FOC_calc.h"

#include "interface/timer.h"

#include "PID/PID.h"
#include "interface/current.h"

float theta = 0.0f;
float voltage_amp = 0.07f; // 電圧振幅 0.8 (最大1.0だが安全マージン)

static PID pid_current_d;
static PID pid_current_q;
static PID pid_q;


extern float encoder_sin;
extern float encoder_cos;
float q_target = 0.0f;
uint16_t encoder_target = 0;

extern FOC_DQ adc_currents_dq;

volatile int32_t revolution = 0;

int32_t prev_encoder = 0;
int16_t encoder_diff = 0;
FOC_DQ dq_pid_out;


static const PIDConfig PIDCONFIG_CURRENT = {
		0.05f,
		5.0f,
		0.0f,
		0.00005f,
		-10.0f,
		10.0f,
		-1.0f,
		1.0f,
		-1.0f,
		1.0f,
		-0.5f,
		0.5f,
		0.0f,
		0.0f
};

static const PIDConfig PIDCONFIG_Q = {
		0.2f,
		0.8f,
		0.0f,
		0.0001f,
		-1000.0f,
		1000.0f,
		-20.0f,
		20.0f,
		-3.0f,
		3.0f,
		-10.0f,
		10.0f,
		-1.0f,
		1.0f,
};

void motor_controller_setup() {
    PID_init(&pid_current_d, &PIDCONFIG_CURRENT);
    PID_init(&pid_current_q, &PIDCONFIG_CURRENT);
    PID_init(&pid_q, &PIDCONFIG_Q);
}

void rps_task() {
	int16_t absolute_position = __HAL_TIM_GET_COUNTER(&htim8);
	encoder_diff = absolute_position - prev_encoder;
	q_target = PID_calc(&pid_q, encoder_target, (float)encoder_diff);
	prev_encoder = absolute_position;
}

void MotorControlTask(){
	static const float adv_learning_rate = 0.00005f * 100.0f;
	static float adv_coef = 0.0f;
	FOC_DQ _dq_current = adc_currents_dq;


	dq_pid_out.d = PID_calc(&pid_current_d, 0, _dq_current.d);
	dq_pid_out.q = PID_calc(&pid_current_q, q_target, _dq_current.q);

	if (dq_pid_out.q > 0.05f || dq_pid_out.q < -0.05f) {
		adv_coef -= adv_learning_rate * dq_pid_out.d;
		if (adv_coef < 0.0f) adv_coef = 0.0f;
	}

	float adv_angle = dq_pid_out.q * adv_coef;
	float adv_sin = arm_sin_f32(adv_angle);
	float adv_cos = arm_cos_f32(adv_angle);
    FOC_DQ dq_out = {0};
	dq_out.d = dq_pid_out.d * adv_cos - dq_pid_out.q * adv_sin;
	dq_out.q = dq_pid_out.d * adv_sin + dq_pid_out.q * adv_cos;

	float norm;
	arm_sqrt_f32(dq_out.d * dq_out.d + dq_out.q * dq_out.q, &norm);
	if (norm > 1.0f) {
		dq_out.d /= norm;
		dq_out.q /= norm;
	}
    const FOC_AB ab_out = {0};
    const FOC_UVW uvw_out = {0};
	FOC_DQtoAB(&dq_out, encoder_sin, encoder_cos, &ab_out);
	FOC_ABtoUVW(&ab_out, &uvw_out);
	timer_setDuty(uvw_out.u, uvw_out.v, uvw_out.w);
}
