#ifndef MOTORS_DRIVER_H
#define MOTORS_DRIVER_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>

typedef enum {
    PWM_SRC_TIM,
    PWM_SRC_PCA9685
} pwm_source_t;

typedef struct {
    GPIO_TypeDef* IN1_port;
    uint16_t IN1_pin;

    GPIO_TypeDef* IN2_port;
    uint16_t IN2_pin;

    pwm_source_t pwm_src;

    union {
        struct {
            TIM_HandleTypeDef* htim;
            uint32_t channel;
        } tim;

        struct {
            uint8_t channel;
        } pca;
    } pwm;

    bool enabled;
} drvMotor_t;

void DriverMotor_Init(drvMotor_t* motor);
void DriverMotor_Enable(drvMotor_t* motor);
void DriverMotor_Disable(drvMotor_t* motor);
void DriverMotor_setDirection(drvMotor_t* motor, bool forward);
void DriverMotor_setPwm(drvMotor_t* motor, uint16_t pwm);

#endif
