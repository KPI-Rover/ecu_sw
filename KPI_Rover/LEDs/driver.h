#ifndef DRIVER_H
#define DRIVER_H

#include "stm32f4xx_hal.h"

typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} Led_t;

void LedDriver_Init(Led_t* leds, uint8_t count);
void LedDriver_On(uint8_t ledIndex);
void LedDriver_Off(uint8_t ledIndex);
void LedDriver_Blink(uint8_t ledIndex, uint16_t times);
void LedDriver_Flash(uint8_t ledIndex, uint16_t times);

#endif // DRIVER_H
