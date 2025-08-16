#include "stm32f4xx_hal.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "driver.h"

#define BUZZER_BLOCKING_TIME_LIMIT 1000

#define CHECK_ERROR(condition, err_value) do { \
		if (condition) { \
			errors = (err_value); \
			goto fail; \
		} \
	} while (0)

enum BuzzerState {
	BUZZER_ON,
	BUZZER_OFF,
	BUZZER_NOT_ACTIVE,
	IMPOSSIBLE_STATE
};

struct BuzzerStateMemory {
	TickType_t on_duration;
	TickType_t off_duration;
	TickType_t total_active_time;
	TickType_t active_time_limit;
	enum BuzzerState current_state;
};

static GPIO_TypeDef *GPIO_buzzer_port;
static uint16_t GPIO_buzzer_pin;
static int buzzer_initialized;

static TimerHandle_t timer_handle;
static StaticTimer_t timer;
static struct BuzzerStateMemory bsm;

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

static void Buzzer_ResetTimer(void)
{
	if (timer_handle == NULL)
		return;

	(void) xTimerStop(timer_handle, BUZZER_BLOCKING_TIME_LIMIT);
}

unsigned int Buzzer_ConfigurePort(const GPIO_TypeDef * const gpio_port, const uint16_t gpio_pin)
{
	Buzzer_ResetTimer();

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

static unsigned int Buzzer_SetON(void)
{
	unsigned int errors = 0;

	CHECK_ERROR(!buzzer_initialized, BUZZER_NOT_INITIALIZED_ERROR);

	HAL_GPIO_WritePin(GPIO_buzzer_port, GPIO_buzzer_pin, GPIO_PIN_SET);

fail:
	return errors;
}

static unsigned int Buzzer_SetOFF(void)
{
	unsigned int errors = 0;

	CHECK_ERROR(!buzzer_initialized, BUZZER_NOT_INITIALIZED_ERROR);

	HAL_GPIO_WritePin(GPIO_buzzer_port, GPIO_buzzer_pin, GPIO_PIN_RESET);

fail:
	return errors;
}

unsigned int Buzzer_Enable(void)
{
	Buzzer_ResetTimer();
	return Buzzer_SetON();
}

unsigned int Buzzer_Disable(void)
{
	Buzzer_ResetTimer();
	return Buzzer_SetOFF();
}

static void Buzzer_TimerCallback(TimerHandle_t t)
{
	TickType_t new_period;

	switch (bsm.current_state) {
	case BUZZER_ON:
		Buzzer_SetOFF(); // disable first, determine action course next
		bsm.current_state = BUZZER_OFF;
		bsm.total_active_time += bsm.on_duration;

		new_period = bsm.total_active_time + bsm.off_duration <= bsm.active_time_limit
			? bsm.off_duration
			: bsm.active_time_limit - bsm.total_active_time;

		if (!new_period)
			break;

		(void) xTimerChangePeriod(timer_handle, new_period, 0);
		(void) xTimerStart(timer_handle, 0);
		break;

	case BUZZER_OFF:
		bsm.current_state = BUZZER_ON;
		bsm.total_active_time += bsm.off_duration;

		new_period = bsm.total_active_time + bsm.on_duration <= bsm.active_time_limit
			? bsm.on_duration
			: bsm.active_time_limit - bsm.total_active_time;

		if (!new_period)
			break;

		(void) xTimerChangePeriod(timer_handle, new_period, 0);
		(void) xTimerStart(timer_handle, 0);
		Buzzer_SetON();
		break;

	case BUZZER_NOT_ACTIVE:
	case IMPOSSIBLE_STATE:
	default:
		// why are we here then?
		break;
	}
}

unsigned int Buzzer_Pulse(const unsigned int on_time_ms, const unsigned int period_time_ms, const unsigned int total_active_time_ms)
{
	Buzzer_ResetTimer();

	unsigned int errors = 0;

	CHECK_ERROR(!buzzer_initialized, BUZZER_NOT_INITIALIZED_ERROR);

	const TickType_t on_time_ticks = pdMS_TO_TICKS(on_time_ms);
	CHECK_ERROR(!on_time_ticks, BUZZER_ZERO_ONTIME_ERROR);

	const TickType_t off_time_ticks = pdMS_TO_TICKS(period_time_ms) - on_time_ticks;
	CHECK_ERROR(!off_time_ticks, BUZZER_ZERO_OFFTIME_ERROR);

	const TickType_t active_time_limit = pdMS_TO_TICKS(total_active_time_ms);
	CHECK_ERROR(!active_time_limit, BUZZER_ZERO_ACTIVE_TIME_ERROR);

	if (timer_handle == NULL) {
		timer_handle = xTimerCreateStatic(
				"",
				on_time_ticks,
				pdFALSE,
				0,
				Buzzer_TimerCallback,
				&timer);
	} else {
		CHECK_ERROR(xTimerChangePeriod(timer_handle, on_time_ticks, BUZZER_BLOCKING_TIME_LIMIT) == pdFAIL, BUZZER_TIMER_BUSY);
	}

	CHECK_ERROR(xTimerStart(timer_handle, BUZZER_BLOCKING_TIME_LIMIT) == pdFAIL, BUZZER_TIMER_BUSY);
	Buzzer_SetON();

	bsm.on_duration = on_time_ticks;
	bsm.off_duration = off_time_ticks;
	bsm.total_active_time = 0;
	bsm.active_time_limit = active_time_limit;
	bsm.current_state = BUZZER_ON;

	return 0;

fail:
	Buzzer_ResetTimer();
	Buzzer_SetOFF();
	return errors;
}
