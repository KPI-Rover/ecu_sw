#ifndef MOTORS_DRIVER_H
#define MOTORS_DRIVER_H

#include "stm32f4xx_hal.h"

typedef enum {
    MOTOR_STATE_STOP = 0,
    MOTOR_STATE_FORWARD,
    MOTOR_STATE_BACKWARD,
    MOTOR_STATE_BRAKE
} MotorState_t;


typedef enum {
    MOTOR_SUBSTATE_NONE = 0,
    MOTOR_SUBSTATE_ACCEL,
    MOTOR_SUBSTATE_CONSTANT,
    MOTOR_SUBSTATE_DECEL
} MotorSubState_t;


typedef struct {
	GPIO_TypeDef* IN1_port;
	uint16_t IN1_pin;
	GPIO_TypeDef* IN2_port;
	uint16_t IN2_pin;
	TIM_HandleTypeDef* htim_pwm;
	uint32_t pwm_channel;

	MotorState_t state;
	MotorSubState_t substate;
	uint32_t state_entry_time_ms;
	uint8_t target_speed;
	uint8_t current_speed;
} Motor_HandleTypeDef;

void DriverMotors_Init(Motor_HandleTypeDef* motor);
void DriverMotors_Forward(Motor_HandleTypeDef* motor);
void DriverMotors_Backward(Motor_HandleTypeDef* motor);
void DriverMotors_Stop(Motor_HandleTypeDef* motor);
void DriverMotors_SetSpeed(Motor_HandleTypeDef* motor, uint8_t speed_percent);
void DriverMotors_TimerTask(Motor_HandleTypeDef* motor);

#endif
