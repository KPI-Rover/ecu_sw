#include "ulGD.h"
#include <math.h>
#include <stdbool.h>

void ulGD_Init(ulGD_t* gd)
{
    /* Tuning Speed */
    gd->alpha_p = 0.005f;   // Kp learning rate
    gd->alpha_i = 0.0005f;  // Ki learning rate

    /* Safety Limits */
    gd->kp_min = 0.040f;
	gd->kp_max = 0.080f;

	gd->ki_min = 0.010f;
	gd->ki_max = 0.020f;

    /* Stability Settings */
    gd->max_delta = 0.0001f; // Max step size per cycle
    gd->deadzone  = 0.02f;   // Ignore small errors

    /* Anti-Drift (Leakage) */
    gd->kp_anchor = 0.046f;  // Target Kp (Base value)
    gd->ki_anchor = 0.013f;  // Target Ki (Base value)
    gd->leakage_rate = 0.01f;// Return-to-base force
}

void ulGD_Update(ulGD_t* gd, float error, float error_integral, float pwm_norm, float* kp, float* ki)
{
    /* Validation */
    if (isnan(error) || isnan(error_integral)) return;
    if (isnan(*kp) || isnan(*ki)) return;

    /* Skip if error is negligible */
    bool inside_deadzone = fabsf(error) < gd->deadzone;

    /* Skip if motor is saturated (0% or 100%) */
    if (pwm_norm < 0.05f || pwm_norm > 0.95f) return;

    float dKp = 0.0f;
    float dKi = 0.0f;

    /* Calculate Gradients */
    if (!inside_deadzone) {
        // P-term gradient
        dKp = gd->alpha_p * error * fabsf(error);

        // I-term gradient (with integral windup protection)
        float learning_integral = error_integral;
        if (learning_integral > 50.0f) learning_integral = 50.0f;
        if (learning_integral < -50.0f) learning_integral = -50.0f;

        dKi = gd->alpha_i * error * learning_integral;

        // Clamp step size
        if (dKp > gd->max_delta)  dKp = gd->max_delta;
        if (dKp < -gd->max_delta) dKp = -gd->max_delta;

        if (dKi > gd->max_delta)  dKi = gd->max_delta;
        if (dKi < -gd->max_delta) dKi = -gd->max_delta;
    }

    /* Calculate Leakage (Pull towards anchor) */
    float leak_p = (*kp - gd->kp_anchor) * gd->leakage_rate;
    float leak_i = (*ki - gd->ki_anchor) * gd->leakage_rate;

    /* Update Gains */
    *kp = *kp + dKp - leak_p;
    *ki = *ki + dKi - leak_i;

    /* Apply Hard Limits */
    if (*kp < gd->kp_min) *kp = gd->kp_min;
    if (*kp > gd->kp_max) *kp = gd->kp_max;

    if (*ki < gd->ki_min) *ki = gd->ki_min;
    if (*ki > gd->ki_max) *ki = gd->ki_max;
}
