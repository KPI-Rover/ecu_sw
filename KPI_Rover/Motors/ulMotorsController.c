#include "ulog.h"
#include "cmsis_os.h"
#include <math.h>
#include <Motors/ulMotorsController.h>
#include "Motors/PCA9685.h"

extern TIM_HandleTypeDef htim2;
static osTimerId_t motors_timer_handle;

#define MOTORS_CONTROL_PERIOD_MS 20

ulMotorsController_t g_motors_ctrl;
MotorsCtrlState_t g_motors_state = MOTORS_STATE_INIT;


static void init_motors_hw_mapping(ulMotorsController_t* ctrl)
{
    // Motor 1
    ctrl->motors[0].IN1_port    = GPIOE;
    ctrl->motors[0].IN1_pin     = GPIO_PIN_2;
    ctrl->motors[0].IN2_port    = GPIOE;
    ctrl->motors[0].IN2_pin     = GPIO_PIN_4;
    ctrl->motors[0].pwm_src     = PWM_SRC_PCA9685;
    ctrl->motors[0].pwm.pca.channel = 0;

    // Motor 2
    ctrl->motors[1].IN1_port    = GPIOE;
    ctrl->motors[1].IN1_pin     = GPIO_PIN_5;
    ctrl->motors[1].IN2_port    = GPIOE;
    ctrl->motors[1].IN2_pin     = GPIO_PIN_6;
    ctrl->motors[1].pwm_src     = PWM_SRC_PCA9685;
    ctrl->motors[1].pwm.pca.channel = 1;

}


void ulMotorsController_Init(ulMotorsController_t* ctrl)
{
    ULOG_INFO("MotorsController init");

    init_motors_hw_mapping(ctrl);

    for (int i = 0; i < ULMOTORS_NUM_MOTORS; i++) {

        ulPID_Init(&ctrl->pids[i], 5.0f, 0.015f, 0.0f);
        ulRLS_Init(&ctrl->rls[i]);

        ctrl->last_pwm[i]      = 0.0f;
        ctrl->adapt_counter[i] = 0;

        ctrl->setpoint_rpm[i]  = 0.0f;
        ctrl->measured_rpm[i]  = 0.0f;

        ulPID_SetOutputLimits(&ctrl->pids[i], 0.0f, 1.0f);

        if (ctrl->motors[i].pwm_src == PWM_SRC_TIM) {
            ctrl->pwm_max[i] =
                (float)__HAL_TIM_GET_AUTORELOAD(
                    ctrl->motors[i].pwm.tim.htim);
        }
        else {
            ctrl->pwm_max[i] = 4095.0f;
        }

        ULOG_INFO("Motor[%d] pwm_max=%.0f", i, ctrl->pwm_max[i]);
    }
}



void ulMotorsController_Run(ulMotorsController_t* ctrl)
{
    for (int i = 0; i < ULMOTORS_NUM_MOTORS; i++) {

        float sp  = ctrl->setpoint_rpm[i];
        float rpm = ctrl->measured_rpm[i];

        bool forward = (sp >= 0.0f);
        DriverMotor_setDirection(&ctrl->motors[i], forward);

        float pwm_norm = ulPID_Compute(&ctrl->pids[i],
                                       fabsf(sp),
                                       fabsf(rpm));

        if (pwm_norm < 0.0f) pwm_norm = 0.0f;
        if (pwm_norm > 1.0f) pwm_norm = 1.0f;

        ulRLS_Update(&ctrl->rls[i], rpm, ctrl->last_pwm[i]);

        if (++ctrl->adapt_counter[i] >= 100) {
            ctrl->adapt_counter[i] = 0;

            if (fabsf(sp) > 300.0f && fabsf(rpm) > 100.0f) {
                ulPID_AutoTune_FromRLS(&ctrl->pids[i],
                                       &ctrl->rls[i],
                                       MOTORS_CONTROL_PERIOD_MS / 1000.0f);
            }
        }

        uint32_t pwm_hw =
            (uint32_t)(pwm_norm * ctrl->pwm_max[i]);

        DriverMotor_setPwm(&ctrl->motors[i], pwm_hw);

        ctrl->last_pwm[i] = pwm_norm;

        ULOG_DEBUG("M%d sp=%.0f rpm=%.0f pwm=%lu",
                   i, sp, rpm, pwm_hw);
    }
}

static void MotorsTimerCallback(void *argument)
{
    (void)argument;

    switch (g_motors_state)
    {
        case MOTORS_STATE_INIT:
        	ULOG_INFO("Motors state: INIT -> initializing HW");

			for (int i = 0; i < ULMOTORS_NUM_MOTORS; i++) {
				DriverMotor_Init(&g_motors_ctrl.motors[i]);
			}

			g_motors_state = MOTORS_STATE_IDLE;
			ULOG_INFO("Motors state: INIT -> IDLE");
			break;

        case MOTORS_STATE_IDLE:
        {
            bool need_run = false;
            for (int i = 0; i < ULMOTORS_NUM_MOTORS; i++) {
                if (fabsf(g_motors_ctrl.setpoint_rpm[i]) > 1.0f) {
                    need_run = true;
                    break;
                }
            }

            if (need_run) {
                ULOG_INFO("Motors state: IDLE -> RUN");

                for (int i = 0; i < ULMOTORS_NUM_MOTORS; i++) {
                    DriverMotor_Enable(&g_motors_ctrl.motors[i]);
                }

                g_motors_state = MOTORS_STATE_RUN;
            }
            break;
        }

        case MOTORS_STATE_RUN:
        {
            ulMotorsController_Run(&g_motors_ctrl);

            bool all_zero = true;
            for (int i = 0; i < ULMOTORS_NUM_MOTORS; i++) {
                if (fabsf(g_motors_ctrl.setpoint_rpm[i]) > 1.0f) {
                    all_zero = false;
                    break;
                }
            }
            if (all_zero) {
                ULOG_INFO("Motors state: RUN -> IDLE (all setpoints ~0)");
                for (int i = 0; i < ULMOTORS_NUM_MOTORS; i++) {
                    DriverMotor_setPwm(&g_motors_ctrl.motors[i], 0);
                    g_motors_ctrl.last_pwm[i] = 0.0f;
                }
                g_motors_state = MOTORS_STATE_IDLE;
            }
            break;
        }

        case MOTORS_STATE_ERROR:
        default:
            for (int i = 0; i < ULMOTORS_NUM_MOTORS; i++) {
                DriverMotor_setPwm(&g_motors_ctrl.motors[i], 0);
                g_motors_ctrl.last_pwm[i] = 0.0f;
            }
            break;
    }
}


void ulMotorsController_Task(void* argument)
{
    (void)argument;

    ulMotorsController_Init(&g_motors_ctrl);

    PCA9685_Init();

    const osTimerAttr_t timer_attrs = {
   	   .name = "Motors_Timer"
   	};

   	motors_timer_handle = osTimerNew(MotorsTimerCallback, osTimerPeriodic, NULL, &timer_attrs);
   	if (!motors_timer_handle) {
   	   ULOG_ERROR("Failed to create Motors timer");
   	   osThreadExit();
   	}

   	osDelay(10);

   	if (osTimerStart(motors_timer_handle, MOTORS_CONTROL_PERIOD_MS) != osOK) {
   	   ULOG_ERROR("Failed to start Motors timer");
   	   osThreadExit();
   	}



   	for (;;) {

   			g_motors_ctrl.setpoint_rpm[0] = 1200.0f;
   			g_motors_ctrl.setpoint_rpm[1] = 1200.0f;
   	        g_motors_ctrl.measured_rpm[0] = 1100.0f;
   	        g_motors_ctrl.measured_rpm[1] = 1200.0f;


   	    osDelay(10);
   	}
}
