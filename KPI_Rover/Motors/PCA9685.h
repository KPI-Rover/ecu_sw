#ifndef PCA_9685_H_
#define PCA_9685_H_

#pragma once
#include <stdint.h>

void PCA9685_Init(void);
void PCA9685_SetPWM(uint8_t channel, uint16_t on, uint16_t off);
void PCA9685_SetPin(uint8_t channel, uint8_t val);

#endif // PCA_9685_H_
