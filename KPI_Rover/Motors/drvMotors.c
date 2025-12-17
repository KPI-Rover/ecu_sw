#include <Motors/drvMotors.h>
#include <Motors/PCA9685.h>
#include "ulog.h"

void DriverMotor_Init(drvMotor_t* motor)
{
    if (motor->pwm_src == PWM_SRC_TIM) {
        HAL_TIM_PWM_Start(motor->pwm.tim.htim, motor->pwm.tim.channel);
        __HAL_TIM_MOE_ENABLE(motor->pwm.tim.htim);
    }
    else if (motor->pwm_src == PWM_SRC_PCA9685) {
        PCA9685_SetPWM(motor->pwm.pca.channel, 0, 0);
    }

    motor->enabled = false;
    DriverMotor_setDirection(motor, true);
}

void DriverMotor_Enable(drvMotor_t* motor)
{
    motor->enabled = true;
    DriverMotor_setPwm(motor, 0);
}

void DriverMotor_Disable(drvMotor_t* motor)
{
    motor->enabled = false;
    DriverMotor_setPwm(motor, 0);
}

void DriverMotor_setDirection(drvMotor_t* motor, bool forward){
    HAL_GPIO_WritePin(motor->IN1_port, motor->IN1_pin, forward ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(motor->IN2_port, motor->IN2_pin, forward ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void DriverMotor_setPwm(drvMotor_t* motor, uint16_t pwm)
{
    if (!motor->enabled)
        pwm = 0;

    if (motor->pwm_src == PWM_SRC_TIM) {
        __HAL_TIM_SET_COMPARE(
            motor->pwm.tim.htim,
            motor->pwm.tim.channel,
            pwm
        );
    }
    else if (motor->pwm_src == PWM_SRC_PCA9685) {
        if (pwm > 4095) pwm = 4095;
        PCA9685_SetPWM(motor->pwm.pca.channel, 0, pwm);
    }
}



