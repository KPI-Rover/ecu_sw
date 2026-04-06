#ifndef MOTORS_ULGD_H_
#define MOTORS_ULGD_H_

#include <stdint.h>
#include <math.h>

typedef struct {
    float alpha_p;
    float alpha_i;
    float kp_min, kp_max;
    float ki_min, ki_max;
    float max_delta;
    float deadzone;
    float kp_anchor;
    float ki_anchor;
    float leakage_rate;

} ulGD_t;

void ulGD_Init(ulGD_t* gd);
void ulGD_Update(ulGD_t* gd, float error, float error_integral, float pwm_norm, float* kp, float* ki);

#endif /* MOTORS_ULGD_H_ */
