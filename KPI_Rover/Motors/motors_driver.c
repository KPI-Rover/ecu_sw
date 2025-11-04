#include "motors_driver.h"
#include "ulog.h"

void DriverMotors_Init(Motor_HandleTypeDef* motor){
	HAL_TIM_PWM_Start(motor->htim_pwm, motor->pwm_channel);
	__HAL_TIM_MOE_ENABLE(motor->htim_pwm);
	DriverMotors_Stop(motor);
}

void DriverMotors_Forward(Motor_HandleTypeDef* motor){
	HAL_GPIO_WritePin(motor->IN1_port, motor->IN1_pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(motor->IN2_port, motor->IN2_pin, GPIO_PIN_RESET);
}

void DriverMotors_Backward(Motor_HandleTypeDef* motor){
	HAL_GPIO_WritePin(motor->IN1_port, motor->IN1_pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(motor->IN2_port, motor->IN2_pin, GPIO_PIN_SET);
}

void DriverMotors_Stop(Motor_HandleTypeDef* motor){
	HAL_GPIO_WritePin(motor->IN1_port, motor->IN1_pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(motor->IN2_port, motor->IN2_pin, GPIO_PIN_RESET);
}

void DriverMotors_SetSpeed(Motor_HandleTypeDef* motor, uint8_t speed_percent) {
    if (speed_percent > 100) speed_percent = 100;
    uint32_t period = __HAL_TIM_GET_AUTORELOAD(motor->htim_pwm);
    uint32_t compare = (uint32_t)(( (uint64_t)(period + 1) * speed_percent ) / 100);
    __HAL_TIM_SET_COMPARE(motor->htim_pwm, motor->pwm_channel, compare);
    ULOG_INFO("SetSpeed %u%% -> compare=%lu period=%lu", speed_percent, compare, period);
}

