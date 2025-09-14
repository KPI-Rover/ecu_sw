#include <stddef.h>

#include "stm32f4xx_hal.h"

#include "driver.h"

#define BUZZER_BLOCKING_TIME_LIMIT 1000

#define CHECK_ERROR(condition, err_value) do { \
		if (condition) { \
			errors = (err_value); \
			goto fail; \
		} \
	} while (0)

static unsigned int count_bits_16bit(uint16_t value)
{
	unsigned int result = 0;

	for ( ; value; value >>= 1)
		if (value & 1)
			result++;

	return result;
}

unsigned int Buzzer_ConfigurePort(struct BuzzerObject * const self, const GPIO_TypeDef * const gpio_port, const uint16_t gpio_pin)
{
	self->bsm.current_state = BUZZER_NOT_TIMED;

	unsigned int errors = 0;

	CHECK_ERROR(count_bits_16bit(gpio_pin) != 1, BUZZER_PIN_ERROR);

	self->GPIO_buzzer_port = (GPIO_TypeDef *) gpio_port;
	self->GPIO_buzzer_pin = (uint16_t) gpio_pin;

	self->buzzer_initialized = 1;

	return errors;

fail:
	self->buzzer_initialized = 0;
	return errors;
}

static unsigned int Buzzer_SetON(struct BuzzerObject * const self)
{
	unsigned int errors = 0;

	CHECK_ERROR(!self->buzzer_initialized, BUZZER_NOT_INITIALIZED_ERROR);

	HAL_GPIO_WritePin(self->GPIO_buzzer_port, self->GPIO_buzzer_pin, GPIO_PIN_SET);

fail:
	return errors;
}

static unsigned int Buzzer_SetOFF(struct BuzzerObject * const self)
{
	unsigned int errors = 0;

	CHECK_ERROR(!self->buzzer_initialized, BUZZER_NOT_INITIALIZED_ERROR);

	HAL_GPIO_WritePin(self->GPIO_buzzer_port, self->GPIO_buzzer_pin, GPIO_PIN_RESET);

fail:
	return errors;
}

unsigned int Buzzer_Enable(struct BuzzerObject * const self)
{
	self->bsm.current_state = BUZZER_NOT_TIMED;
	return Buzzer_SetON(self);
}

unsigned int Buzzer_Disable(struct BuzzerObject * const self)
{
	self->bsm.current_state = BUZZER_NOT_TIMED;
	return Buzzer_SetOFF(self);
}

void Buzzer_TimerTask(struct BuzzerObject * const self)
{
	if (self->bsm.current_state == BUZZER_NOT_TIMED)
		return;

	self->bsm.total_active_time_left--;
	self->bsm.current_state_valid_for--;

	if (!self->bsm.total_active_time_left) {
		Buzzer_SetOFF(self);
		self->bsm.current_state = BUZZER_NOT_TIMED;
		return;
	}

	if (self->bsm.current_state_valid_for)
		return;

	switch (self->bsm.current_state) {
	case BUZZER_ON:
		Buzzer_SetOFF(self); // disable first, determine action course next
		self->bsm.current_state = BUZZER_OFF;
		self->bsm.current_state_valid_for = self->bsm.off_duration;
		break;

	case BUZZER_OFF:
		self->bsm.current_state = BUZZER_ON;
		self->bsm.current_state_valid_for = self->bsm.on_duration;
		Buzzer_SetON(self);
		break;

	case BUZZER_NOT_TIMED:
	case IMPOSSIBLE_STATE:
	default:
		// how did we get here?
		break;
	}
}

unsigned int Buzzer_Pulse(struct BuzzerObject * const self, const unsigned int on_time_ms, const unsigned int period_time_ms, const unsigned int total_active_time_ms)
{
	self->bsm.current_state = BUZZER_NOT_TIMED;

	unsigned int errors = 0;

	CHECK_ERROR(!self->buzzer_initialized, BUZZER_NOT_INITIALIZED_ERROR);

	const int on_time_ticks = on_time_ms / 10;
	CHECK_ERROR(!on_time_ticks, BUZZER_ZERO_ONTIME_ERROR);

	const int off_time_ticks = (period_time_ms / 10) - on_time_ticks;
	CHECK_ERROR(!off_time_ticks, BUZZER_ZERO_OFFTIME_ERROR);

	const int active_time_limit = total_active_time_ms / 10;
	CHECK_ERROR(!active_time_limit, BUZZER_ZERO_ACTIVE_TIME_ERROR);

	Buzzer_SetON(self);

	self->bsm.on_duration = on_time_ticks;
	self->bsm.off_duration = off_time_ticks;
	self->bsm.total_active_time_left = active_time_limit;
	self->bsm.current_state_valid_for = on_time_ticks;
	self->bsm.current_state = BUZZER_ON;

	return 0;

fail:
	self->bsm.current_state = BUZZER_NOT_TIMED;
	Buzzer_SetOFF(self);
	return errors;
}
