#ifndef MOTORS_DRIVER_H
#define MOTORS_DRIVER_H

#include "stm32f4xx_hal.h"

typedef struct {
	GPIO_TypeDef* IN1_port;
	uint16_t IN1_pin;
	GPIO_TypeDef* IN2_port;
	uint16_t IN2_pin;
	TIM_HandleTypeDef* htim_pwm;
	uint32_t pwm_channel;
} Motor_HandleTypeDef;

void DriverMotors_Init(Motor_HandleTypeDef* motor);
void DriverMotors_Forward(Motor_HandleTypeDef* motor);
void DriverMotors_Backward(Motor_HandleTypeDef* motor);
void DriverMotors_Stop(Motor_HandleTypeDef* motor);
void DriverMotors_SetSpeed(Motor_HandleTypeDef* motor, uint8_t speed_percent);

#endif
