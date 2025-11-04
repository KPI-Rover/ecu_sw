#include "motors_manager.h"
#include "cmsis_os.h"
#include "ulog.h"

extern TIM_HandleTypeDef htim1;

#define MOTOR_TIMER_PERIOD_MS   50u

static osTimerId_t motor_timer_handle;
static Motor_HandleTypeDef motor1;

static void motor_timer_callback(void *argument);


void MotorManager_Init(void) {

    motor1.IN1_port = GPIOC;
    motor1.IN1_pin  = GPIO_PIN_4;
    motor1.IN2_port = GPIOC;
    motor1.IN2_pin  = GPIO_PIN_5;
    motor1.htim_pwm = &htim1;
    motor1.pwm_channel = TIM_CHANNEL_1;

    DriverMotors_Init(&motor1);

    const osTimerAttr_t timer_attrs = {
		.name = "Motor_Timer"
	};

	motor_timer_handle = osTimerNew(motor_timer_callback, osTimerPeriodic, NULL, &timer_attrs);
	if (!motor_timer_handle) {
		ULOG_ERROR("Failed to create Motor timer");
		osThreadExit();
	}

	osDelay(10);

	if (osTimerStart(motor_timer_handle, MOTOR_TIMER_PERIOD_MS) != osOK) {
		ULOG_ERROR("Failed to start Motor timer");
		osThreadExit();
	}
}

void MotorManager_Task(void* argument) {
	MotorManager_Init();
    while(1) {

    	MotorManager_MoveForward(100);
    	ULOG_INFO("MoveForward");
    	osDelay(4000);

    	MotorManager_Stop();
    	osDelay(1000);

    	MotorManager_MoveBackward(55);
    	ULOG_INFO("MoveBackward");
    	osDelay(3000);

    	MotorManager_MoveForward(80);
		ULOG_INFO("MoveForward");
		osDelay(3000);

    	MotorManager_Stop();
    	osDelay(1000);

    }
}

static void motor_timer_callback(void *argument) {
    DriverMotors_TimerTask(&motor1);
}


void MotorManager_MoveForward(uint8_t speed) {
	if (motor1.substate == MOTOR_SUBSTATE_DECEL) {
		ULOG_INFO("Can't move forward while decelerating");
		return;
	}
	DriverMotors_Forward(&motor1);
	motor1.target_speed = speed;
	motor1.current_speed = 0;
	motor1.state = MOTOR_STATE_FORWARD;
	motor1.substate = MOTOR_SUBSTATE_ACCEL;
	motor1.state_entry_time_ms = HAL_GetTick();
}

void MotorManager_MoveBackward(uint8_t speed) {
	if (motor1.substate == MOTOR_SUBSTATE_DECEL) {
		ULOG_INFO("Can't move backward while decelerating");
		return;
	}
	DriverMotors_Backward(&motor1);
	motor1.target_speed = speed;
	motor1.current_speed = 0;
	motor1.state = MOTOR_STATE_BACKWARD;
	motor1.substate = MOTOR_SUBSTATE_ACCEL;
	motor1.state_entry_time_ms = HAL_GetTick();
}

void MotorManager_Stop(void) {
    if (motor1.state == MOTOR_STATE_FORWARD || motor1.state == MOTOR_STATE_BACKWARD) {
        motor1.substate = MOTOR_SUBSTATE_DECEL;
    } else {
        DriverMotors_Stop(&motor1);
        motor1.state = MOTOR_STATE_STOP;
        motor1.substate = MOTOR_SUBSTATE_NONE;
    }
}
