#ifndef __BUZZER_DRIVER
#define __BUZZER_DRIVER

#include "stm32f4xx_hal.h"

#define BUZZER_PIN_ERROR 0x1U
#define BUZZER_NOT_INITIALIZED_ERROR 0x2U
#define BUZZER_HAL_ERROR 0x3U
#define BUZZER_VALUE_ERROR 0x4U
#define BUZZER_ZERO_ONTIME_ERROR 0x5U
#define BUZZER_ZERO_OFFTIME_ERROR 0x6U
#define BUZZER_ZERO_ACTIVE_TIME_ERROR 0x7U
#define BUZZER_TIMER_BUSY 0x8U

enum BuzzerState {
	BUZZER_NOT_TIMED,
	BUZZER_ON,
	BUZZER_OFF,
	IMPOSSIBLE_STATE
};

struct BuzzerStateMemory {
	uint32_t on_duration;
	uint32_t off_duration;
	uint32_t total_active_time_left;
	uint32_t current_state_valid_for;
	enum BuzzerState current_state;
};

struct BuzzerObject {
	GPIO_TypeDef *GPIO_buzzer_port;
	uint16_t GPIO_buzzer_pin;
	int8_t buzzer_initialized;
	struct BuzzerStateMemory bsm;
};

uint32_t Buzzer_ConfigurePort(struct BuzzerObject * const self, const GPIO_TypeDef * const gpio_port, const uint16_t gpio_pin);
uint32_t Buzzer_Enable(struct BuzzerObject * const self);
uint32_t Buzzer_Disable(struct BuzzerObject * const self);
uint32_t Buzzer_Pulse(struct BuzzerObject * const self, const uint32_t on_time_ms, const uint32_t period_time_ms, const uint32_t total_active_time_ms);
void Buzzer_TimerTask(struct BuzzerObject * const self);

#endif /* __BUZZER_DRIVER */
