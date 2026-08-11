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

extern FOC_DQ adc_currents_dq;

volatile int32_t revolution = 0;

int32_t prev_encoder = 0;
int16_t encoder_diff = 0;

void HAL_TIMEx_EncoderIndexCallback(TIM_HandleTypeDef *htim)
{
    if (__HAL_TIM_IS_TIM_COUNTING_DOWN(htim))
        revolution--;
    else
        revolution++;
}


static const PIDConfig PIDCONFIG_CURRENT = {
		0.03f,
		20.0f,
		0.0f,
		0.00005f,
		-10.0f,
		10.0f,
		-1.0f,
		1.0f,
		-1.0f,
		1.0f,
		-5.0f,
		5.0f,
		0.0f,
		0.0f
};

static const PIDConfig PIDCONFIG_Q = {
		0.3f,
		0.5f,
		0.0f,
		0.0001f,
		-1000.0f,
		1000.0f,
		-2.0f,
		20.0f,
		-1.0f,
		1.0f,
		-5.0f,
		5.0f,
		0.0f,
		0.0f
};

void motor_controller_setup() {
    PID_init(&pid_current_d, &PIDCONFIG_CURRENT);
    PID_init(&pid_current_q, &PIDCONFIG_CURRENT);
    PID_init(&pid_q, &PIDCONFIG_Q);
}

void khz_task() {
	int32_t absolute_position =revolution * 4096 + __HAL_TIM_GET_COUNTER(&htim8);
	encoder_diff =	(absolute_position - prev_encoder) * -1;
	q_target = PID_calc(&pid_q, 100, (float)encoder_diff);
	prev_encoder = absolute_position;
}

void MotorControlTask() {
    theta += 0.008f; 
    if (theta > 6.283185f) theta -= 6.283185f;

    // 2. 電圧ベクトル生成 (逆Park変換の簡易版)
    // 本来は Id, Iq から計算しますが、テストなので直接 Alpha, Beta を生成
    float valpha = voltage_amp * cosf(theta);
    float vbeta  = voltage_amp * sinf(theta);
    
    // 1. 逆Clarke変換 (alpha, beta -> U, V, W)
    // Va = Valpha
    // Vb = -0.5 * Valpha + (sqrt(3)/2) * Vbeta
    // Vc = -0.5 * Valpha - (sqrt(3)/2) * Vbeta
    
    // sqrt(3)/2 = 0.8660254f
    float Va = valpha;
    float Vb = -0.5f * valpha + 0.8660254f * vbeta;
    float Vc = -0.5f * valpha - 0.8660254f * vbeta;

    // 2. Min-Max法によるSVPWM (コモンモード電圧の注入)
    // 3相の中で一番大きい電圧と小さい電圧を見つける
    float V_max = fmaxf(Va, fmaxf(Vb, Vc));
    float V_min = fminf(Va, fminf(Vb, Vc));
    
    // 鞍型にするためのオフセット電圧
    float V_common = -0.5f * (V_max + V_min);

    // 3. オフセットを加算してデューティ比に変換
    // 入力(-1.0~1.0) -> CCR(0~ARR)
    // Center Alignedなので、duty 0.0 が中心、-1.0が0、+1.0がARRになるようにマップする
    // 式: CCR = ( (V + V_common) + 1.0 ) * 0.5 * ARR
    
    // クリップ処理 (過変調防止)
    // ※厳密には正規化が必要ですが、テストなので簡易リミッタで
    float u_out = Va + V_common;
    float v_out = Vb + V_common;
    float w_out = Vc + V_common;
    
    // レジスタ書き込み
    TIM1->CCR1 = (uint32_t)((u_out + 1.0f) * 0.5f * PWM_PERIOD_COUNTS);
    TIM1->CCR2 = (uint32_t)((v_out + 1.0f) * 0.5f * PWM_PERIOD_COUNTS);
    TIM1->CCR3 = (uint32_t)((w_out + 1.0f) * 0.5f * PWM_PERIOD_COUNTS);
}
