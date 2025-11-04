#ifndef MOTORS_MANAGER_H
#define MOTORS_MANAGER_H

#include "motors_driver.h"


void MotorManager_Init(void);
void MotorManager_Task(void* argument);
void MotorManager_MoveForward(uint8_t speed);
void MotorManager_MoveBackward(uint8_t speed);
void MotorManager_Stop(void);

#endif
