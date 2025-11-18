#ifndef UL_MOTORS_CONTROLLER_H
#define UL_MOTORS_CONTROLLER_H

#include <stdint.h>
#include "motors_driver.h"
#include "ul_pid.h"
#include "ul_rls.h"

#define ULMOTORS_NUM_MOTORS 2

typedef struct {
    drvMotor_t motors[ULMOTORS_NUM_MOTORS];
    ulPID_t    pids[ULMOTORS_NUM_MOTORS];
    ulRLS_t    rls[ULMOTORS_NUM_MOTORS];

    float      last_pwm[ULMOTORS_NUM_MOTORS];
    uint16_t   adapt_counter[ULMOTORS_NUM_MOTORS];

    float      pwm_max[ULMOTORS_NUM_MOTORS];

    uint16_t   period_ms;

    float measured_rpm[ULMOTORS_NUM_MOTORS];
    float setpoint_rpm[ULMOTORS_NUM_MOTORS];
    float scale_filtered[ULMOTORS_NUM_MOTORS];
} ulMotorsController_t;

typedef enum
{
    MOTORS_STATE_INIT = 0,
    MOTORS_STATE_IDLE,
    MOTORS_STATE_RUN,
    MOTORS_STATE_ERROR
} MotorsCtrlState_t;

extern ulMotorsController_t g_motors_ctrl;

extern MotorsCtrlState_t g_motors_state;


void ulMotorsController_Init(ulMotorsController_t* ctrl);
void ulMotorsController_Run(ulMotorsController_t* ctrl);
void ulMotorsController_Task(void* argument);

void ulMotorsController_RetunePid(ulMotorsController_t* ctrl, int i);

#endif // UL_MOTORS_CONTROLLER_H
