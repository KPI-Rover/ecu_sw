#include <math.h>
#include <Motors/ulPID.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

// Gain for dynamic anti-windup (how fast to reduce integral when saturated)
#define PID_AW_GAIN   0.5f

void ulPID_Init(ulPID_t* pid, float kp, float ki, float kd){
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->integral     = 0.0f;
    pid->prev_meas    = 0.0f;
    pid->d_filtered   = 0.0f;

    /* Output limits (usually 0.0 to 1.0 for PWM) */
    pid->out_min = 0.0f;
    pid->out_max = 1.0f;

    /* Integral limits (prevent indefinite error accumulation) */
    pid->int_min = -1.0f;
    pid->int_max =  1.0f;

    /* Setpoint filter (1.0 = no filtering, lower = smoother target) */
    pid->sp_filtered = 0.0f;
    pid->sp_alpha    = 1.0f;

    /* Derivative filter (CRITICAL for noisy encoders, 0.05-0.2 is good) */
    pid->d_alpha    = 0.2f;

    /* Max change of output per second (Soft Start) */
    pid->slew_rate = 100.0f;

    /* Static friction compensation */
    pid->deadzone = 0.0f;

    pid->last_output = 0.0f;

    // pid->last_time is not used inside Compute anymore, but good to init
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

/**
 * Main PID calculation loop.
 * @param dt: Delta time in seconds (must be constant for stability!)
 */
float ulPID_Compute(ulPID_t* pid, float setpoint, float measured, float dt)
{
    /* Safety: Avoid division by zero */
    if (dt <= 0.000001f) {
        return pid->last_output;
    }

    /* Initialization (Bumpless Transfer) */
    /* Prevents "jumps" when PID is called for the first time */
    if (!pid->initialized)
    {
        pid->sp_filtered = setpoint;
        pid->prev_meas   = measured;
        pid->d_filtered  = 0.0f;
        pid->last_output = 0.0f;
        pid->initialized = true;
    }

    /* Setpoint Filtering (Low Pass Filter) */
    /* Smooths out jerky changes in target speed */
    pid->sp_filtered += pid->sp_alpha * (setpoint - pid->sp_filtered);
    float sp = pid->sp_filtered;

    /* Error Calculation */
    float error = sp - measured;

    /* Integral Term (Accumulate Error) */
    pid->integral += error * dt;

    // Hard clamp for integral to prevent overflow
    if (pid->integral > pid->int_max) pid->integral = pid->int_max;
    if (pid->integral < pid->int_min) pid->integral = pid->int_min;

    /* Derivative Term (Derivative on Measurement) */
    /* We calculate slope of Sensor data, NOT Error. Prevents spikes when Setpoint changes. */
    float d_meas = (measured - pid->prev_meas) / dt;
    pid->prev_meas = measured;

    // Note the minus sign: d(Error)/dt = -d(Measured)/dt (assuming constant setpoint)
    float d_raw = -d_meas;

    // Filter the noisy D-term (Low Pass Filter)
    pid->d_filtered += pid->d_alpha * (d_raw - pid->d_filtered);

    /* PID Sum */
    float out_unsat =
        pid->kp * error +
        pid->ki * pid->integral +
        pid->kd * pid->d_filtered;

    /* Saturation (Clamping) */
    float out_clamped = out_unsat;
    if (out_clamped > pid->out_max) out_clamped = pid->out_max;
    if (out_clamped < pid->out_min) out_clamped = pid->out_min;

    /* Dynamic Anti-Windup */
    /* If output is clamped, reduce Integral to stop it from growing uselessly */
    if (pid->ki != 0.0f && out_clamped != out_unsat)
    {
        float aw = out_clamped - out_unsat;
        pid->integral += (PID_AW_GAIN * aw * dt);
    }

    float actuator = out_clamped;

    /* Deadzone Compensation (Static Friction) */
    /* Adds a minimum power to overcome motor friction */
    if (fabsf(actuator) > 0.001f)
    {
        if (actuator > 0.0f)
            actuator += pid->deadzone;
        else
            actuator -= pid->deadzone;

        // Re-clamp after adding deadzone
        if (actuator > pid->out_max) actuator = pid->out_max;
        if (actuator < pid->out_min) actuator = pid->out_min;
    }

    /* Slew Rate Limiter (Soft Start / Anti-Shock) */
    /* Limits how fast the output can change to protect gears/electronics */
    float diff = actuator - pid->last_output;
    float max_change = pid->slew_rate * dt;

    if (diff > max_change)
        actuator = pid->last_output + max_change;
    else if (diff < -max_change)
        actuator = pid->last_output - max_change;

    pid->last_output = actuator;

    return actuator;
}
