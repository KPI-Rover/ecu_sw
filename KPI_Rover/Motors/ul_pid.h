#ifndef UL_PID_H
#define UL_PID_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

struct ulRLS_s;
typedef struct ulRLS_s ulRLS_t;

typedef struct {
    float kp;
    float ki;
    float kd;

    float integral;
    float prev_meas;
    float d_filtered;

    float out_min;
    float out_max;

    float int_min;
    float int_max;

    float sp_filtered;
    float sp_alpha;

    float d_alpha;
	float last_output;
	float slew_rate;

	float deadzone;

	uint32_t last_time;

    bool initialized;
} ulPID_t;

void ulPID_Init(ulPID_t* pid, float kp, float ki, float kd);
void ulPID_Reset(ulPID_t* pid);
void ulPID_SetParams(ulPID_t* pid, float kp, float ki, float kd);
void ulPID_SetOutputLimits(ulPID_t* pid, float out_min, float out_max);
float ulPID_Compute(ulPID_t* pid, float setpoint, float measured);
void ulPID_AutoTune_FromRLS(ulPID_t* pid, const ulRLS_t* rls, float Ts);
void ulPID_ResetI(ulPID_t* pid);
#endif // UL_PID_H
