#include "driver.h"
#include "cmsis_os.h"
#include <stdlib.h>
#include <string.h>

#define MAX_LEDS 7

static LedSettings_t ledsInitialized[MAX_LEDS];
static uint8_t ledCount = 0;

static LedRuntime_t runtimes[MAX_LEDS];

static inline uint32_t now_ms(void) { return HAL_GetTick(); }

void LedDriver_Init(LedSettings_t *self, uint8_t count) {
	if (self == NULL || count == 0) return;

    if (count > MAX_LEDS) count = MAX_LEDS;
    ledCount = count;

    memset(runtimes, 0, sizeof(runtimes));

    for (uint8_t i = 0; i < count; i++) {
    	if (self[i].port == NULL) continue;
    	ledsInitialized[i] = self[i];
    	runtimes[i].state = LED_STATE_OFF;
    	runtimes[i].used = 1;
    	runtimes[i].output_on = 0;
        HAL_GPIO_WritePin(ledsInitialized[i].port, ledsInitialized[i].pin, GPIO_PIN_RESET);
    }
}

void LedDriver_On(uint8_t ledIndex) {
	if (ledIndex >= ledCount || !runtimes[ledIndex].used || runtimes[ledIndex].output_on) return;
    runtimes[ledIndex].state = LED_STATE_ON;
    runtimes[ledIndex].output_on = 1;
    HAL_GPIO_WritePin(ledsInitialized[ledIndex].port, ledsInitialized[ledIndex].pin, GPIO_PIN_SET);
}

void LedDriver_Off(uint8_t ledIndex) {
    if (ledIndex >= ledCount || !runtimes[ledIndex].used || !runtimes[ledIndex].output_on) return;
    runtimes[ledIndex].state = LED_STATE_OFF;
    runtimes[ledIndex].output_on = 0;
    HAL_GPIO_WritePin(ledsInitialized[ledIndex].port, ledsInitialized[ledIndex].pin, GPIO_PIN_RESET);
}

void LedDriver_Blink(uint8_t ledIndex, uint16_t times) {
    if (ledIndex >= ledCount || !runtimes[ledIndex].used) return;

    runtimes[ledIndex].state = LED_STATE_BLINK;
    runtimes[ledIndex].times = times;
    runtimes[ledIndex].remaining = times;
    runtimes[ledIndex].on_time_ms = 500;
    runtimes[ledIndex].off_time_ms = 500;
    runtimes[ledIndex].last_time_ms = now_ms();
    runtimes[ledIndex].output_on = 1;
    HAL_GPIO_WritePin(ledsInitialized[ledIndex].port, ledsInitialized[ledIndex].pin, GPIO_PIN_SET);
}

void LedDriver_Flash(uint8_t ledIndex, uint16_t times) {
    if (ledIndex >= ledCount || !runtimes[ledIndex].used) return;

    runtimes[ledIndex].state = LED_STATE_FLASH;
    runtimes[ledIndex].times = times;
    runtimes[ledIndex].remaining = times;
    runtimes[ledIndex].on_time_ms = 100;
    runtimes[ledIndex].off_time_ms = 900;
    runtimes[ledIndex].last_time_ms = now_ms();
    runtimes[ledIndex].output_on = 1;
    HAL_GPIO_WritePin(ledsInitialized[ledIndex].port, ledsInitialized[ledIndex].pin, GPIO_PIN_SET);
}

void LedDriver_TimerTask(void) {
    uint32_t t = now_ms();

    for (uint8_t i = 0; i < ledCount; ++i) {
        if (!runtimes[i].used) continue;

        LedRuntime_t *r = &runtimes[i];

        switch (r->state) {
            case LED_STATE_IDLE:
                // something
                break;

            case LED_STATE_ON:
                if (!r->output_on) {
                    HAL_GPIO_WritePin(ledsInitialized[i].port, ledsInitialized[i].pin, GPIO_PIN_SET);
                    r->output_on = 1;
                }
                break;

            case LED_STATE_OFF:
                if (r->output_on) {
                    HAL_GPIO_WritePin(ledsInitialized[i].port, ledsInitialized[i].pin, GPIO_PIN_RESET);
                    r->output_on = 0;
                }
                break;

            case LED_STATE_BLINK:
            	// skip to FLASH case
            case LED_STATE_FLASH:
                uint32_t elapsed = (t - r->last_time_ms);
                if (r->output_on) {
                    if (elapsed >= r->on_time_ms) {
                       HAL_GPIO_WritePin(ledsInitialized[i].port, ledsInitialized[i].pin, GPIO_PIN_RESET);
                       r->output_on = 0;
                       r->last_time_ms = t;
                       if (r->remaining > 0) {
                           r->remaining--;
                           if (r->remaining == 0) {
                               r->state = LED_STATE_OFF;
                           }
                       }
                    }
                } else {
                    if (elapsed >= r->off_time_ms) {
                       HAL_GPIO_WritePin(ledsInitialized[i].port, ledsInitialized[i].pin, GPIO_PIN_SET);
                       r->output_on = 1;
                       r->last_time_ms = t;
                    }
                }
                break;

            default:
                break;
        }
    }
}
