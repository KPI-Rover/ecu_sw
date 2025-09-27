#ifndef DRIVER_H
#define DRIVER_H

#include "stm32f4xx_hal.h"

typedef enum {
	LED_OFF,
	LED_ON,
	LED_BLINK,
	LED_FLASH
} LedMode_e;

typedef struct {
	GPIO_TypeDef* port;
	uint16_t pin;
	LedMode_e mode;
    uint16_t times;
} LedSettings_t;

typedef struct {
    uint8_t index;
    uint16_t times;
    uint16_t on_time;
    uint16_t off_time;
} LedParams_t;

void LedDriver_Init(LedSettings_t* self, uint8_t count);
void LedDriver_On(uint8_t ledIndex);
void LedDriver_Off(uint8_t ledIndex);
void LedDriver_Blink(uint8_t ledIndex, uint16_t times);
void LedDriver_Flash(uint8_t ledIndex, uint16_t times);

#endif // DRIVER_H
