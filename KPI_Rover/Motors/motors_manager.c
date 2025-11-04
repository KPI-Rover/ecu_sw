#include "motors_manager.h"
#include "cmsis_os.h"
#include "ulog.h"

extern TIM_HandleTypeDef htim1;

static Motor_HandleTypeDef motor1;

void MotorManager_Init(void) {

    motor1.IN1_port = GPIOC;
    motor1.IN1_pin  = GPIO_PIN_4;
    motor1.IN2_port = GPIOC;
    motor1.IN2_pin  = GPIO_PIN_5;
    motor1.htim_pwm = &htim1;
    motor1.pwm_channel = TIM_CHANNEL_1;

    DriverMotors_Init(&motor1);
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

    	MotorManager_Stop();
    	osDelay(1000);

    }
}

void MotorManager_MoveForward(uint8_t speed) {
	DriverMotors_Forward(&motor1);
	DriverMotors_SetSpeed(&motor1, speed);
}

void MotorManager_MoveBackward(uint8_t speed) {
	DriverMotors_Backward(&motor1);
	DriverMotors_SetSpeed(&motor1, speed);
}

void MotorManager_Stop(void) {
	DriverMotors_Stop(&motor1);
}
