#ifndef HAL_MOTOR_H
#define HAL_MOTOR_H

#include "main.h"

void SetMotorPWM(TIM_HandleTypeDef* htim, uint32_t channel, uint16_t duty);
void SetMotorDirection(int motor, int direction);

#endif
