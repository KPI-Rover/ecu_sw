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

void DriverMotors_TimerTask(Motor_HandleTypeDef* motor) {
    uint32_t now = HAL_GetTick();
    uint32_t elapsed = now - motor->state_entry_time_ms;

    switch (motor->state) {

        case MOTOR_STATE_STOP:

            break;

        case MOTOR_STATE_FORWARD:
            switch (motor->substate) {
                case MOTOR_SUBSTATE_ACCEL:
                    if (motor->current_speed < motor->target_speed) {
                        motor->current_speed += 15;
                        DriverMotors_SetSpeed(motor, motor->current_speed);
                    } else {
                        motor->substate = MOTOR_SUBSTATE_CONSTANT;
                    }
                    break;

                case MOTOR_SUBSTATE_CONSTANT:

                    break;

                case MOTOR_SUBSTATE_DECEL:
                    if (motor->current_speed > 0) {
                        if (motor->current_speed > 15)
                            motor->current_speed -= 15;
                        else
                            motor->current_speed = 0;

                        DriverMotors_SetSpeed(motor, motor->current_speed);
                    } else {
                        DriverMotors_SetSpeed(motor, 0);
                        DriverMotors_Stop(motor);
                        motor->state = MOTOR_STATE_STOP;
                        motor->substate = MOTOR_SUBSTATE_NONE;
                        ULOG_INFO("Motor stopped completely");
                    }
                    break;


                default:
                    break;
            }
            break;

        case MOTOR_STATE_BACKWARD:
        	switch (motor->substate) {
				case MOTOR_SUBSTATE_ACCEL:
					if (motor->current_speed < motor->target_speed) {
						motor->current_speed += 15;
						DriverMotors_SetSpeed(motor, motor->current_speed);
					} else {
						motor->substate = MOTOR_SUBSTATE_CONSTANT;
					}
					break;

				case MOTOR_SUBSTATE_CONSTANT:

					break;

				case MOTOR_SUBSTATE_DECEL:
				    if (motor->current_speed > 0) {
				        if (motor->current_speed > 15)
				            motor->current_speed -= 15;
				        else
				            motor->current_speed = 0;

				        DriverMotors_SetSpeed(motor, motor->current_speed);
				    } else {
				        DriverMotors_SetSpeed(motor, 0);
				        DriverMotors_Stop(motor);
				        motor->state = MOTOR_STATE_STOP;
				        motor->substate = MOTOR_SUBSTATE_NONE;
				        ULOG_INFO("Motor stopped completely");
				    }
				    break;


				default:
					break;
			}
            break;

        default:
            break;
    }
}

