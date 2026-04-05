#include "ulog.h"
#include "cmsis_os.h"
#include <math.h>
#include <Motors/ulMotorsController.h>
#include "Motors/PCA9685.h"
#include <Database/ulDatabase.h>
#include <stdio.h>
#include <string.h>
#include "usbd_cdc_if.h"

static osTimerId_t motors_timer_handle;

#define MOTORS_CONTROL_PERIOD_MS 5

const float dt_sec = MOTORS_CONTROL_PERIOD_MS / 1000.0f;

ulMotorsController_t g_motors_ctrl;
MotorsCtrlState_t g_motors_state = MOTORS_STATE_INIT;

static const uint16_t key_rpm[ULMOTORS_NUM_MOTORS] = {
    MOTOR_FL_RPM,
    MOTOR_FR_RPM,
    MOTOR_RL_RPM,
    MOTOR_RR_RPM
};

static const uint16_t key_kp[ULMOTORS_NUM_MOTORS] = {
	MOTOR_FL_KP,
	MOTOR_FR_KP,
	MOTOR_RL_KP,
	MOTOR_RR_KP
};

static const uint16_t key_ki[ULMOTORS_NUM_MOTORS] = {
	MOTOR_FL_KI,
	MOTOR_FR_KI,
	MOTOR_RL_KI,
	MOTOR_RR_KI
};

static const uint16_t key_kd[ULMOTORS_NUM_MOTORS] = {
	MOTOR_FL_KD,
	MOTOR_FR_KD,
	MOTOR_RL_KD,
	MOTOR_RR_KD
};

static const uint16_t key_setpoint[ULMOTORS_NUM_MOTORS] = {
	MOTOR_FL_SETPOINT,
	MOTOR_FR_SETPOINT,
	MOTOR_RL_SETPOINT,
	MOTOR_RR_SETPOINT
};

static void Motors_UpdateFromDB_FloatArray(const uint16_t* keys, float* dst, int count)
{
    for (int i = 0; i < count; i++) {
        int32_t val = 0;
        if (ulDatabase_getInt32(keys[i], &val)) {
            dst[i] = (float)val;
        }
    }
}

static void ulMotorsController_UpdateFeedback(ulMotorsController_t* ctrl)
{
    Motors_UpdateFromDB_FloatArray(key_rpm, ctrl->measured_rpm, ULMOTORS_NUM_MOTORS);
}

static void ulMotorsController_UpdateFromDB(ulMotorsController_t* ctrl)
{
    Motors_UpdateFromDB_FloatArray(key_setpoint, ctrl->setpoint_rpm, ULMOTORS_NUM_MOTORS);
}


static void init_motors_hw_mapping(ulMotorsController_t* ctrl)
{
    // MOTOR_1_FL
    ctrl->motors[0].pwm_src     = PWM_SRC_PCA9685;
    ctrl->motors[0].pwm.pca.channel = 0; // MOTOR_1_PWM
    ctrl->motors[0].in1_pca_channel = 1; // MOTOR_1_FWD
    ctrl->motors[0].in2_pca_channel = 2; // MOTOR_1_RVR

    // MOTOR_2_FR
    ctrl->motors[1].pwm_src     = PWM_SRC_PCA9685;
    ctrl->motors[1].pwm.pca.channel = 4; // MOTOR_2_PWM
    ctrl->motors[1].in1_pca_channel = 5; // MOTOR_2_FWD
    ctrl->motors[1].in2_pca_channel = 6; // MOTOR_2_RVR

    // MOTOR_3_RL
	ctrl->motors[2].pwm_src     = PWM_SRC_PCA9685;
    ctrl->motors[2].pwm.pca.channel = 15; // MOTOR_3_PWM
    ctrl->motors[2].in1_pca_channel = 14; // MOTOR_3_FWD
    ctrl->motors[2].in2_pca_channel = 13; // MOTOR_3_RVR

	// MOTOR_4_RR
	ctrl->motors[3].pwm_src     = PWM_SRC_PCA9685;
    ctrl->motors[3].pwm.pca.channel = 11; // MOTOR_4_PWM
    ctrl->motors[3].in1_pca_channel = 10; // MOTOR_4_FWD
    ctrl->motors[3].in2_pca_channel = 9; // MOTOR_4_RVR

}


void ulMotorsController_Init(ulMotorsController_t* ctrl)
{
    ULOG_INFO("MotorsController init");

    init_motors_hw_mapping(ctrl);

    for (int i = 0; i < ULMOTORS_NUM_MOTORS; i++) {

		float kp = 0.0f;
		float ki = 0.0f;
		float kd = 0.0f;

		if (!ulDatabase_getFloat(key_kp[i], &kp)) {
			ULOG_ERROR("Failed to load Kp for motor %d! Using default.", i);
			kp = 0.046f;
		}

		if (!ulDatabase_getFloat(key_ki[i], &ki)) {
			ULOG_ERROR("Failed to load Ki for motor %d!", i);
			ki = 0.013f;
		}

		if (!ulDatabase_getFloat(key_kd[i], &kd)) {
			 kd = 0.0001f;
		}

		ulPID_Init(&ctrl->pids[i], kp, ki, kd);

		ctrl->pids[i].deadzone = 0.05f;
		ctrl->pids[i].slew_rate = 25.0f;
		ctrl->pids[i].d_alpha = 0.05f;

		ulGD_Init(&ctrl->gd[i]);

        ulPID_SetOutputLimits(&ctrl->pids[i], 0.0f, 1.0f);
        ulPID_SetIntegralLimits(&ctrl->pids[i], -50.0f, 50.0f);

        ctrl->last_pwm[i]      = 0.0f;
        ctrl->setpoint_rpm[i]  = 0.0f;
        ctrl->measured_rpm[i]  = 0.0f;


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
    const float MAX_RPM_CONST = 250.0f;

    for (int i = 0; i < ULMOTORS_NUM_MOTORS; i++) {

    	if (isnan(ctrl->pids[i].kp) || ctrl->pids[i].kp < 0.001f) ctrl->pids[i].kp = 0.046f;
    	if (isnan(ctrl->pids[i].ki) || ctrl->pids[i].ki < 0.0f)   ctrl->pids[i].ki = 0.013f;

        float sp  = ctrl->setpoint_rpm[i];
        float rpm = ctrl->measured_rpm[i];

        if (isnan(rpm)) rpm = 0.0f;

        bool forward = (sp >= 0.0f);
        DriverMotor_setDirection(&ctrl->motors[i], forward);

        float pwm_norm = ulPID_Compute(&ctrl->pids[i], fabsf(sp), fabsf(rpm), dt_sec);

        if (isnan(pwm_norm)) {
            pwm_norm = 0.0f;
        }

        if (pwm_norm < 0.0f) pwm_norm = 0.0f;
        if (pwm_norm > 1.0f) pwm_norm = 1.0f;

        float max_val = ctrl->pwm_max[i];
        if (max_val < 1.0f) max_val = 4095.0f;

        uint32_t pwm_hw = (uint32_t)(pwm_norm * max_val);

        if (pwm_hw > (uint32_t)max_val) pwm_hw = (uint32_t)max_val;

        DriverMotor_setPwm(&ctrl->motors[i], pwm_hw);
        ctrl->last_pwm[i] = pwm_norm;


//             ADAPTIVE PI (GRADIENT DESCENT)

        float error = sp - rpm;
        float error_norm = error / MAX_RPM_CONST;

        ulGD_Update(
            &ctrl->gd[i],
            error_norm,
            ctrl->pids[i].integral,
            pwm_norm,
            &ctrl->pids[i].kp,
            &ctrl->pids[i].ki
        );
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
        	ulMotorsController_UpdateFromDB(&g_motors_ctrl);
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
                    ulPID_Reset(&g_motors_ctrl.pids[i]);
                }

                g_motors_state = MOTORS_STATE_RUN;
            }
            break;
        }

        case MOTORS_STATE_RUN:
        {
        	static uint8_t sp_div = 0;

        	if (++sp_div >= 4) {
        	    sp_div = 0;
        	    ulMotorsController_UpdateFromDB(&g_motors_ctrl);
        	}
        	ulMotorsController_UpdateFeedback(&g_motors_ctrl);
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

   	osDelay(5);

   	if (osTimerStart(motors_timer_handle, MOTORS_CONTROL_PERIOD_MS) != osOK) {
   	   ULOG_ERROR("Failed to start Motors timer");
   	   osThreadExit();
   	}


   	for (;;) {

   	    osDelay(5);
   	}
}
