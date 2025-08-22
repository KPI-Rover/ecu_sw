#ifndef __STM32F4XX_HAL_GPIO_H
#define __STM32F4XX_HAL_GPIO_H


#include <stdint.h>

#define GPIO_PIN_0 ((uint16_t)0x0001)
#define GPIO_PIN_1 ((uint16_t)0x0002)
#define GPIO_PIN_2 ((uint16_t)0x0004)
#define GPIO_PIN_3 ((uint16_t)0x0008)
#define GPIO_PIN_4 ((uint16_t)0x0010)
#define GPIO_PIN_5 ((uint16_t)0x0020)
#define GPIO_PIN_6 ((uint16_t)0x0040)
#define GPIO_PIN_7 ((uint16_t)0x0080)
#define GPIO_PIN_8 ((uint16_t)0x0100)
#define GPIO_PIN_9 ((uint16_t)0x0200)
#define GPIO_PIN_10 ((uint16_t)0x0400)
#define GPIO_PIN_11 ((uint16_t)0x0800)
#define GPIO_PIN_12 ((uint16_t)0x1000)
#define GPIO_PIN_13 ((uint16_t)0x2000)
#define GPIO_PIN_14 ((uint16_t)0x4000)
#define GPIO_PIN_15 ((uint16_t)0x8000)
#define GPIO_PIN_All ((uint16_t)0xFFFF)

typedef enum {
	GPIO_PIN_RESET = 0,
	GPIO_PIN_SET
} GPIO_PinState;

typedef struct {
	uint32_t ODR;
} GPIO_TypeDef;

void HAL_GPIO_WritePin(
		GPIO_TypeDef* GPIOx,
		uint16_t GPIO_Pin,
		GPIO_PinState PinState
);


#endif /* __STM32F4XX_HAL_GPIO_H */
