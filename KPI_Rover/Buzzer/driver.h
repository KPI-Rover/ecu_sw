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


enum BuzzerEventType {
	NO_EVENT,
	IMPOSSIBLE_EVENT,
	TO_ON,
	TO_OFF,
	TO_PULSE
};

struct BuzzerEvent {
	enum BuzzerEventType ev;
	uint32_t pulse_on_for;
	uint32_t pulse_off_for;
	uint32_t pulse_total_for;
};


enum BuzzerGlobalStateType {
	IMPOSSIBLE_STATE,
	BUZZER_OFF,
	BUZZER_ON,
	BUZZER_PULSE
};

struct BuzzerGlobalState {
	enum BuzzerGlobalStateType current_state;
	uint32_t current_state_since;
};


enum BuzzerPulseSubStateType {
	BUZZER_TO_ON,
	BUZZER_DELAY_BEFORE_OFF,
	BUZZER_TO_OFF,
	BUZZER_DELAY_BEFORE_ON,
	BUZZER_FINAL_DELAY
};

struct BuzzerPulseConfig {
	uint32_t pulse_on_for;
	uint32_t pulse_off_for;
	uint32_t pulse_total_for;
};

struct BuzzerPulseSubState {
	enum BuzzerPulseSubStateType current_state;
	struct BuzzerPulseConfig config;
	uint32_t current_state_since;
	uint32_t state_lifetime_override;
};


struct BuzzerConfig {
	GPIO_TypeDef *GPIO_port;
	uint16_t GPIO_pin;
	int8_t initialized;
};

struct BuzzerObject {
	struct BuzzerConfig c;
	struct BuzzerGlobalState s;
	struct BuzzerPulseSubState ps;
	struct BuzzerEvent e;
};

uint32_t Buzzer_ConfigurePort(struct BuzzerObject * const self, const GPIO_TypeDef * const gpio_port, const uint16_t gpio_pin);
uint32_t Buzzer_Enable(struct BuzzerObject * const self);
uint32_t Buzzer_Disable(struct BuzzerObject * const self);
uint32_t Buzzer_Pulse(struct BuzzerObject * const self, const uint32_t on_time_ms, const uint32_t period_time_ms, const uint32_t total_active_time_ms);
void Buzzer_TimerTask(struct BuzzerObject * const self);

#endif /* __BUZZER_DRIVER */
