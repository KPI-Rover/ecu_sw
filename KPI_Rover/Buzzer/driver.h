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

unsigned int Buzzer_ConfigurePort(const GPIO_TypeDef * const gpio_port, const uint16_t gpio_pin);
unsigned int Buzzer_Enable(void);
unsigned int Buzzer_Disable(void);
unsigned int Buzzer_Pulse(const unsigned int on_time_ms, const unsigned int period_time_ms, const unsigned int total_active_time_ms);

#endif /* __BUZZER_DRIVER */
