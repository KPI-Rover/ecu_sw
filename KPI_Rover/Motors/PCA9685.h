#pragma once
#include <stdint.h>

void PCA9685_Init(void);
void PCA9685_SetPWM(uint8_t channel, uint16_t on, uint16_t off);
