#ifndef MOTORS_DRIVER_H
#define MOTORS_DRIVER_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>

typedef struct {
	GPIO_TypeDef* IN1_port;
	    uint16_t IN1_pin;

	    GPIO_TypeDef* IN2_port;
	    uint16_t IN2_pin;

	    TIM_HandleTypeDef* htim_pwm;
	    uint32_t pwm_channel;

	    bool enabled;
} drvMotor_t;

void DriverMotor_Init(drvMotor_t* motor);
void DriverMotor_Enable(drvMotor_t* motor);
void DriverMotor_Disable(drvMotor_t* motor);
void DriverMotor_setDirection(drvMotor_t* motor, bool forward);
void DriverMotor_setPwm(drvMotor_t* motor, uint16_t pwm);

#endif
