#include "ul_motors_controller.h"
#include "ulog.h"
#include "cmsis_os.h"
#include <math.h>

extern TIM_HandleTypeDef htim1;
static osTimerId_t motors_timer_handle;

#define MOTORS_CONTROL_PERIOD_MS 20

ulMotorsController_t g_motors_ctrl;
MotorsCtrlState_t g_motors_state = MOTORS_STATE_INIT;


static void init_motors_hw_mapping(ulMotorsController_t* ctrl)
{
    // Motor 0
    ctrl->motors[0].IN1_port    = GPIOC;
    ctrl->motors[0].IN1_pin     = GPIO_PIN_4;
    ctrl->motors[0].IN2_port    = GPIOC;
    ctrl->motors[0].IN2_pin     = GPIO_PIN_5;
    ctrl->motors[0].htim_pwm    = &htim1;
    ctrl->motors[0].pwm_channel = TIM_CHANNEL_1;

    // Motor 1
    ctrl->motors[1].IN1_port    = GPIOE;
    ctrl->motors[1].IN1_pin     = GPIO_PIN_7;
    ctrl->motors[1].IN2_port    = GPIOE;
    ctrl->motors[1].IN2_pin     = GPIO_PIN_8;
    ctrl->motors[1].htim_pwm    = &htim1;
    ctrl->motors[1].pwm_channel = TIM_CHANNEL_2;
}


void ulMotorsController_Init(ulMotorsController_t* ctrl)
{
    ULOG_INFO("ENTER ulMotorsController_Init");

    init_motors_hw_mapping(ctrl);

    for (int i = 0; i < ULMOTORS_NUM_MOTORS; i++) {

        float kp = 5.0f, ki = 0.015f, kd = 0.0f;
        ulPID_Init(&ctrl->pids[i], kp, ki, kd);

        ulRLS_Init(&ctrl->rls[i]);

        ctrl->last_pwm[i]       = 0.0f;
        ctrl->adapt_counter[i]  = 0;
        ctrl->pwm_max[i]        = 0.0f;

        ctrl->setpoint_rpm[i]   = 0.0f;
        ctrl->measured_rpm[i]   = 0.0f;
        ctrl->scale_filtered[i] = 1.0f;


        uint32_t arr = __HAL_TIM_GET_AUTORELOAD(ctrl->motors[i].htim_pwm);
        ulPID_SetOutputLimits(&ctrl->pids[i], 0.0f, (float)arr);
        ctrl->pwm_max[i] = (float)arr;

        ULOG_INFO("Motor %d init: PID(Kp=%.3f Ki=%.3f Kd=%.3f), ARR=%lu, pwm_ch=%lu",
                  i, kp, ki, kd, arr, ctrl->motors[i].pwm_channel);
    }
    //g_motors_state = MOTORS_STATE_IDLE;

}


void ulMotorsController_Run(ulMotorsController_t* ctrl)
{
    for (int i = 0; i < ULMOTORS_NUM_MOTORS; i++) {

        float setpoint = ctrl->setpoint_rpm[i];
        float rpm      = ctrl->measured_rpm[i];

        bool forward = (setpoint >= 0.0f);
        DriverMotor_setDirection(&ctrl->motors[i], forward);

        float pwm = ulPID_Compute(&ctrl->pids[i], setpoint, rpm);


        if (pwm < 0.0f) pwm = 0.0f;
        if (pwm > ctrl->pwm_max[i]) pwm = ctrl->pwm_max[i];

        ulRLS_Update(&ctrl->rls[i], rpm, ctrl->last_pwm[i]);

        if (++ctrl->adapt_counter[i] >= 100) {
            ctrl->adapt_counter[i] = 0;

            float setpoint = ctrl->setpoint_rpm[i];

            if (fabsf(setpoint) > 300.0f && fabsf(rpm) > 100.0f) {

                ulPID_AutoTune_FromRLS(&ctrl->pids[i],
                                       &ctrl->rls[i],
                                       MOTORS_CONTROL_PERIOD_MS / 1000.0f);

                ULOG_INFO("Motor[%d] autotune: Kp=%.3f Ki=%.3f a=%.3f b=%.3f",
                          i,
                          ctrl->pids[i].kp,
                          ctrl->pids[i].ki,
                          ctrl->rls[i].theta[0],
                          ctrl->rls[i].theta[1]);
            }
        }

        DriverMotor_setPwm(&ctrl->motors[i], (uint16_t)pwm);
        ctrl->last_pwm[i] = pwm;

        ULOG_DEBUG("MOTOR[%d] sp=%d rpm=%d pwm=%d dir=%s",
                   i, (int)setpoint, (int)rpm, (int)pwm,
                   forward ? "FWD" : "BWD");
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
   	        g_motors_ctrl.measured_rpm[0] = 1150.0f;
   	        g_motors_ctrl.measured_rpm[1] = 1150.0f;



   	    osDelay(10);
   	}
}
