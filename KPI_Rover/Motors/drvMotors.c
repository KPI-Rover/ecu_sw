#include <Motors/drvMotors.h>
#include "ulog.h"

void DriverMotor_Init(drvMotor_t* motor){
	HAL_TIM_PWM_Start(motor->htim_pwm, motor->pwm_channel);
	__HAL_TIM_MOE_ENABLE(motor->htim_pwm);

	DriverMotor_setPwm(motor, 0);
	DriverMotor_setDirection(motor, true);
	motor->enabled = false;

	ULOG_INFO("drvMotor_init: PWM on ch=%lu", motor->pwm_channel);
}

void DriverMotor_Enable(drvMotor_t* motor){
	motor->enabled = true;
	ULOG_INFO("Motor enabled (ch=%lu)", motor->pwm_channel);
}

void DriverMotor_Disable(drvMotor_t* motor){
	DriverMotor_setPwm(motor, 0);
    motor->enabled = false;
    ULOG_INFO("Motor disabled (ch=%lu)", motor->pwm_channel);
}

void DriverMotor_setDirection(drvMotor_t* motor, bool forward){
    HAL_GPIO_WritePin(motor->IN1_port, motor->IN1_pin, forward ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(motor->IN2_port, motor->IN2_pin, forward ? GPIO_PIN_RESET : GPIO_PIN_SET);

    ULOG_DEBUG("Motor dir=%s", forward ? "FWD" : "BWD");
}

void DriverMotor_setPwm(drvMotor_t* motor, uint16_t pwm)
{
    if (!motor->enabled)
        return;

    __HAL_TIM_SET_COMPARE(motor->htim_pwm, motor->pwm_channel, pwm);
    //ULOG_DEBUG("Motor PWM set %u (ch=%lu)", pwm, motor->pwm_channel);
}


