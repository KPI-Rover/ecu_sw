#include "ul_pid.h"
#include <math.h>
#include <stdbool.h>
#include "ul_rls.h"
#include "stm32f4xx_hal.h"

#define PID_AW_GAIN   0.2f

void ulPID_Init(ulPID_t* pid, float kp, float ki, float kd){
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->integral     = 0.0f;
    pid->prev_meas    = 0.0f;
    pid->d_filtered   = 0.0f;

    pid->out_min = 0.0f;
    pid->out_max = 1000.0f;

    pid->int_min = -500.0f;
    pid->int_max =  500.0f;

    pid->sp_filtered = 0.0f;
    pid->sp_alpha    = 0.25f;

    pid->d_alpha    = 0.4f;
    pid->slew_rate = 20.0f;
    pid->deadzone = 300.0f;

    pid->last_output = 0.0f;

    pid->last_time   = HAL_GetTick();
    pid->initialized = false;
}

void ulPID_Reset(ulPID_t* pid){
    pid->integral    = 0.0f;
    pid->prev_meas   = 0.0f;
    pid->d_filtered  = 0.0f;
    pid->last_output = 0.0f;
    pid->sp_filtered = 0.0f;
    pid->initialized = false;
}

void ulPID_ResetI(ulPID_t* pid){
    pid->integral = 0.0f;
}

void ulPID_SetParams(ulPID_t* pid, float kp, float ki, float kd){
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
}

void ulPID_SetOutputLimits(ulPID_t* pid, float out_min, float out_max){
    pid->out_min = out_min;
    pid->out_max = out_max;
}

void ulPID_SetIntegralLimits(ulPID_t* pid, float int_min, float int_max){
    pid->int_min = int_min;
    pid->int_max = int_max;
}

float ulPID_Compute(ulPID_t* pid, float setpoint, float measured){
    uint32_t now = HAL_GetTick();
    float dt = (now - pid->last_time) / 1000.0f;
    pid->last_time = now;

    if (dt <= 0.0f) dt = 0.001f;
    if (dt > 0.1f) dt = 0.1f;

    if (!pid->initialized)
    {
        pid->sp_filtered = setpoint;
        pid->prev_meas   = measured;
        pid->d_filtered  = 0.0f;
        pid->last_output = 0.0f;
        pid->initialized = true;
    }

    pid->sp_filtered += pid->sp_alpha * (setpoint - pid->sp_filtered);
    float sp = pid->sp_filtered;
    float error = sp - measured;

    pid->integral += error * dt;
    if (pid->integral > pid->int_max) pid->integral = pid->int_max;
    if (pid->integral < pid->int_min) pid->integral = pid->int_min;

    float d_meas = (measured - pid->prev_meas) / dt;
    pid->prev_meas = measured;

    float d_raw = -d_meas;
    pid->d_filtered += pid->d_alpha * (d_raw - pid->d_filtered);

    float out_unsat =
        pid->kp * error +
        pid->ki * pid->integral +
        pid->kd * pid->d_filtered;

    float out_clamped = out_unsat;
    if (out_clamped > pid->out_max) out_clamped = pid->out_max;
    if (out_clamped < pid->out_min) out_clamped = pid->out_min;

    if (pid->ki != 0.0f)
    {
        float aw = out_clamped - out_unsat;
        pid->integral += (PID_AW_GAIN * aw) / pid->ki;

        if (pid->integral > pid->int_max) pid->integral = pid->int_max;
        if (pid->integral < pid->int_min) pid->integral = pid->int_min;
    }

    float actuator = out_clamped;

	if (fabsf(actuator) > 0.0f)
	{
		if (actuator > 0.0f)
			actuator += pid->deadzone;
		else
			actuator -= pid->deadzone;

		if (actuator > pid->out_max) actuator = pid->out_max;
		if (actuator < pid->out_min) actuator = pid->out_min;
	}

    float diff = actuator - pid->last_output;
    float max_change = pid->slew_rate * dt;

    if (diff > max_change)
        actuator = pid->last_output + max_change;
    else if (diff < -max_change)
        actuator = pid->last_output - max_change;

    pid->last_output = actuator;

    return actuator;
}

void ulPID_AutoTune_FromRLS(ulPID_t* pid, const ulRLS_t* rls, float Ts){
    float a = rls->theta[0];
    float b = rls->theta[1];

    if (a > 0.999f) a = 0.999f;
    if (a < 0.01f)  a = 0.01f;

    float tau = -Ts / logf(a);
    float K = b / (1.0f - a);

    if (K < 0.01f) K = 0.01f;
    if (K > 1000.0f) K = 1000.0f;

    float lambda = 0.25f * tau;
    if (lambda < 0.001f) lambda = 0.001f;

    float kp = tau / (K * lambda);
    float ki = 1.0f / (K * lambda);

    if (kp < 0.0f) kp = 0.0f;
    if (kp > 300.0f) kp = 300.0f;

    if (ki < 0.0f) ki = 0.0f;
    if (ki > 300.0f) ki = 300.0f;

    pid->kp = kp;
    pid->ki = ki;
    pid->kd = 0.0f;
}
