#include <stddef.h>

#include "stm32f4xx_hal.h"

#include "generic.h"
#include "driver.h"

#define BUZZER_BLOCKING_TIME_LIMIT 1000

#define CHECK_ERROR(condition, err_value) do { \
		if (condition) { \
			errors = (err_value); \
			goto fail; \
		} \
	} while (0)


static uint8_t count_bits_16bit(uint16_t value)
{
	uint8_t result = 0;

	for ( ; value; value >>= 1)
		result += value & 1;

	return result;
}

static uint32_t Buzzer_SetON(struct BuzzerObject * const self)
{
	uint32_t errors = 0;

	CHECK_ERROR(!self->c.initialized, BUZZER_NOT_INITIALIZED_ERROR);

	HAL_GPIO_WritePin(self->c.GPIO_port, self->c.GPIO_pin, GPIO_PIN_SET);

fail:
	return errors;
}

static uint32_t Buzzer_SetOFF(struct BuzzerObject * const self)
{
	uint32_t errors = 0;

	CHECK_ERROR(!self->c.initialized, BUZZER_NOT_INITIALIZED_ERROR);

	HAL_GPIO_WritePin(self->c.GPIO_port, self->c.GPIO_pin, GPIO_PIN_RESET);

fail:
	return errors;
}


static void Buzzer_OFF_Enter(struct BuzzerObject * const self, const struct BuzzerEvent * const ev)
{
	Buzzer_SetOFF(self);
}

static void Buzzer_OFF_Handle(struct BuzzerObject * const self)
{

}

static void Buzzer_OFF_Leave(struct BuzzerObject * const self)
{
	Buzzer_SetOFF(self);
}


static void Buzzer_ON_Enter(struct BuzzerObject * const self, const struct BuzzerEvent * const ev)
{
	Buzzer_SetON(self);
}

static void Buzzer_ON_Handle(struct BuzzerObject * const self)
{

}

static void Buzzer_ON_Leave(struct BuzzerObject * const self)
{
	Buzzer_SetON(self);
}


static void Buzzer_PULSE_Enter(struct BuzzerObject * const self, const struct BuzzerEvent * const ev)
{
	self->ps.config.pulse_on_for = ev->pulse_on_for;
	self->ps.config.pulse_off_for = ev->pulse_off_for;
	self->ps.config.pulse_total_for = ev->pulse_total_for;

	self->ps.current_state = BUZZER_TO_ON;
}

static void Buzzer_PULSE_Handle(struct BuzzerObject * const self)
{
	uint32_t tss;

	switch (self->ps.current_state) {
	case BUZZER_TO_ON:
		Buzzer_SetON(self);
		self->ps.current_state_since = HAL_GetTick();

		tss = ticks_elapsed_since(self->s.current_state_since);

		if ((tss + self->ps.config.pulse_on_for) >= self->ps.config.pulse_total_for) {
			self->ps.current_state = BUZZER_FINAL_DELAY;
			self->ps.state_lifetime_override = self->ps.config.pulse_total_for - tss;
		} else {
			self->ps.current_state = BUZZER_DELAY_BEFORE_OFF;
		}

		break;
	case BUZZER_DELAY_BEFORE_OFF:
		if (ticks_elapsed_since(self->ps.current_state_since) >= self->ps.config.pulse_on_for)
			self->ps.current_state = BUZZER_TO_OFF;

		break;
	case BUZZER_TO_OFF:
		Buzzer_SetOFF(self);
		self->ps.current_state_since = HAL_GetTick();

		tss = ticks_elapsed_since(self->s.current_state_since);

		if ((tss + self->ps.config.pulse_off_for) >= self->ps.config.pulse_total_for) {
			self->ps.current_state = BUZZER_FINAL_DELAY;
			self->ps.state_lifetime_override = self->ps.config.pulse_total_for - tss;
		} else {
			self->ps.current_state = BUZZER_DELAY_BEFORE_ON;
		}

		break;
	case BUZZER_DELAY_BEFORE_ON:
		if (ticks_elapsed_since(self->ps.current_state_since) >= self->ps.config.pulse_off_for)
			self->ps.current_state = BUZZER_TO_ON;

		break;
	case BUZZER_FINAL_DELAY:
		if (ticks_elapsed_since(self->ps.current_state_since) < self->ps.state_lifetime_override)
			break;

		self->e.ev = TO_OFF;
		break;
	}
}

static void Buzzer_PULSE_Leave(struct BuzzerObject * const self)
{
	Buzzer_SetOFF(self);
}


static void Buzzer_Reset(struct BuzzerObject * const self)
{
	switch (self->s.current_state) {
	case BUZZER_OFF:
		Buzzer_OFF_Leave(self);
		break;
	case BUZZER_ON:
		Buzzer_ON_Leave(self);
		break;
	case BUZZER_PULSE:
		Buzzer_PULSE_Leave(self);
		break;
	case IMPOSSIBLE_STATE:
	default:
		break;
	}

	self->s.current_state = BUZZER_OFF;
}

static void Buzzer_ProcessEvent(struct BuzzerObject * const self)
{
	if (self->e.ev == NO_EVENT || self->e.ev == IMPOSSIBLE_EVENT)
		return;

	switch (self->s.current_state) {
	case BUZZER_ON:
		Buzzer_ON_Leave(self);
		break;
	case BUZZER_OFF:
		Buzzer_OFF_Leave(self);
		break;
	case BUZZER_PULSE:
		Buzzer_PULSE_Leave(self);
		break;
	default:
		break;
	}

	switch (self->e.ev) {
	case TO_ON:
		self->s.current_state = BUZZER_ON;
		Buzzer_ON_Enter(self, &(self->e));
		break;
	case TO_OFF:
		self->s.current_state = BUZZER_OFF;
		Buzzer_OFF_Enter(self, &(self->e));
		break;
	case TO_PULSE:
		self->s.current_state = BUZZER_PULSE;
		Buzzer_PULSE_Enter(self, &(self->e));
		break;
	default:
		self->s.current_state = IMPOSSIBLE_STATE;
		break;
	}

	self->s.current_state_since = HAL_GetTick();
	self->e.ev = NO_EVENT;
}


uint32_t Buzzer_ConfigurePort(struct BuzzerObject * const self, const GPIO_TypeDef * const gpio_port, const uint16_t gpio_pin)
{
	Buzzer_Reset(self);

	uint32_t errors = 0;

	CHECK_ERROR(count_bits_16bit(gpio_pin) != 1, BUZZER_PIN_ERROR);

	self->c.GPIO_port = (GPIO_TypeDef *) gpio_port;
	self->c.GPIO_pin = (uint16_t) gpio_pin;

	self->c.initialized = 1;

	return errors;

fail:
	self->c.initialized = 0;
	return errors;
}


uint32_t Buzzer_Enable(struct BuzzerObject * const self)
{
	self->e.ev = TO_ON;
	return 0;
}

uint32_t Buzzer_Disable(struct BuzzerObject * const self)
{
	self->e.ev = TO_OFF;
	return 0;
}

uint32_t Buzzer_Pulse(struct BuzzerObject * const self, const uint32_t on_time_ms, const uint32_t period_time_ms, const uint32_t total_active_time_ms)
{
	uint32_t errors = 0;

	const uint32_t off_time_ms = period_time_ms - on_time_ms;

	CHECK_ERROR(!on_time_ms, BUZZER_ZERO_ONTIME_ERROR);
	CHECK_ERROR(!off_time_ms, BUZZER_ZERO_OFFTIME_ERROR);
	CHECK_ERROR(!total_active_time_ms, BUZZER_ZERO_ACTIVE_TIME_ERROR);

	self->e.pulse_on_for = on_time_ms;
	self->e.pulse_off_for = off_time_ms;
	self->e.pulse_total_for = total_active_time_ms;

	self->e.ev = TO_PULSE;

fail:
	return errors;
}

void Buzzer_TimerTask(struct BuzzerObject * const self)
{
	(void) Buzzer_ProcessEvent(self);

	switch (self->s.current_state) {
	case BUZZER_OFF:
		Buzzer_OFF_Handle(self);
		break;
	case BUZZER_ON:
		Buzzer_ON_Handle(self);
		break;
	case BUZZER_PULSE:
		Buzzer_PULSE_Handle(self);
		break;
	default:
		break;
	}
}
