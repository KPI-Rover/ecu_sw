#include "stm32f4xx_hal.h"

#include "FreeRTOS.h"
#include "task.h"

#include "driver.h"

#define CHECK_ERROR(condition, err_value) do { \
		if (condition) { \
			errors = (err_value); \
			goto fail; \
		} \
	} while (0)

static GPIO_TypeDef *GPIO_buzzer_port;
static uint16_t GPIO_buzzer_pin;
static int buzzer_initialized;

static unsigned int count_bits_16bit(uint16_t value)
{
	unsigned int mask = 0x8000,
		     result = 0;

	while (mask)
	{
		if (value & mask)
			result++;

		mask >>= 1;
	}

	return result;
}

unsigned int Buzzer_ConfigurePort(const GPIO_TypeDef * const gpio_port, const uint16_t gpio_pin)
{
	unsigned int errors = 0;

	CHECK_ERROR(count_bits_16bit(gpio_pin) != 1, BUZZER_PIN_ERROR);

	GPIO_buzzer_port = (GPIO_TypeDef *) gpio_port;
	GPIO_buzzer_pin = (uint16_t) gpio_pin;

	buzzer_initialized = 1;

	return errors;

fail:
	buzzer_initialized = 0;
	return errors;
}

unsigned int Buzzer_Enable(void)
{
	unsigned int errors = 0;

	CHECK_ERROR(!buzzer_initialized, BUZZER_NOT_INITIALIZED_ERROR);

	HAL_GPIO_WritePin(GPIO_buzzer_port, GPIO_buzzer_pin, GPIO_PIN_SET);

fail:
	return errors;
}

unsigned int Buzzer_Disable(void)
{
	unsigned int errors = 0;

	CHECK_ERROR(!buzzer_initialized, BUZZER_NOT_INITIALIZED_ERROR);

	HAL_GPIO_WritePin(GPIO_buzzer_port, GPIO_buzzer_pin, GPIO_PIN_RESET);

fail:
	return errors;
}

unsigned int Buzzer_Pulse(const unsigned int on_time_ms, const unsigned int period_time_ms, const unsigned int total_active_time_ms)
{
	unsigned int errors = 0;

	CHECK_ERROR(!buzzer_initialized, BUZZER_NOT_INITIALIZED_ERROR);

	const TickType_t on_time_ticks = pdMS_TO_TICKS(on_time_ms);
	CHECK_ERROR(!on_time_ticks, BUZZER_ZERO_ONTIME_ERROR);

	const TickType_t off_time_ticks = pdMS_TO_TICKS(period_time_ms) - on_time_ticks;
	CHECK_ERROR(!off_time_ticks, BUZZER_ZERO_OFFTIME_ERROR);

	const TickType_t last_on_time_ticks = pdMS_TO_TICKS(total_active_time_ms % period_time_ms);

	const unsigned int total_beep_amount = total_active_time_ms / period_time_ms;

	for (unsigned int i = 0; i < total_beep_amount; i++)
	{
		Buzzer_Enable();
		vTaskDelay(on_time_ticks);
		Buzzer_Disable();
		vTaskDelay(off_time_ticks);
	}

	if (last_on_time_ticks)
	{
		Buzzer_Enable();
		vTaskDelay(last_on_time_ticks);
		Buzzer_Disable();
	}

fail:
	Buzzer_Disable();
	return errors;
}
