#include <string.h>

#include "stm32f4xx_hal_gpio.h"

static GPIO_TypeDef GPIOs[11]; // GPIOA - GPIOK
GPIO_TypeDef *GPIOD;

void gpio_init(void)
{
	// Notice: GPIO A & B registers have specific non-zero values at startup
	memset(GPIOs, 0, sizeof(GPIOs));

	GPIOD = &(GPIOs[3]);
}

uint32_t gpio_check_health(void)
{
	return 0;
}

void HAL_GPIO_WritePin(
		GPIO_TypeDef* GPIOx,
		uint16_t GPIO_Pin,
		GPIO_PinState PinState)
{
	if (PinState == GPIO_PIN_RESET)
		GPIOx->ODR &= ~GPIO_Pin;
	else
		GPIOx->ODR |= GPIO_Pin;
}
