#include "driver.h"
#include "cmsis_os.h"
#include <stdlib.h>

#define MAX_LEDS 4

typedef struct {
    uint8_t index;
    uint16_t times;
    uint16_t on_time;
    uint16_t off_time;
} LedParams_t;

static Led_t leds[MAX_LEDS];
static uint8_t ledCount = 0;

void LedDriver_Init(Led_t* inputLeds, uint8_t count) {
	if (inputLeds == NULL || count == 0) return;

    if (count > MAX_LEDS) count = MAX_LEDS;
    ledCount = count;

    for (uint8_t i = 0; i < count; i++) {
    	if (inputLeds[i].port == NULL) continue;
        leds[i] = inputLeds[i];
        HAL_GPIO_WritePin(leds[i].port, leds[i].pin, GPIO_PIN_RESET);
    }
}

void LedDriver_On(uint8_t ledIndex) {
    if (leds[ledIndex].port == NULL) return;
    if (ledIndex < ledCount)
        HAL_GPIO_WritePin(leds[ledIndex].port, leds[ledIndex].pin, GPIO_PIN_SET);
}

void LedDriver_Off(uint8_t ledIndex) {
    if (leds[ledIndex].port == NULL) return;
    if (ledIndex < ledCount)
        HAL_GPIO_WritePin(leds[ledIndex].port, leds[ledIndex].pin, GPIO_PIN_RESET);
}

static void LedControlTask(void* argument) {
    LedParams_t* params = (LedParams_t*)argument;

    uint16_t count;

    for (;;) {
    	count = params->times;
		do {
			LedDriver_On(params->index);
			osDelay(params->on_time);
			LedDriver_Off(params->index);
			osDelay(params->off_time);
		} while (count == 0 || --count > 0);

		osDelay(2000);
    }
}

void LedDriver_Blink(uint8_t ledIndex, uint16_t times) {
    if (ledIndex >= ledCount || leds[ledIndex].port == NULL) return;

    LedParams_t* params = pvPortMalloc(sizeof(LedParams_t));
    if (params == NULL) return;

    params->index = ledIndex;
    params->times = times;
    params->on_time = 500;  // 50% duty cycle: 500ms ON
    params->off_time = 500; // 50% duty cycle: 500ms OFF

    xTaskCreate(LedControlTask, "LedBlink", 128, params, osPriorityHigh, NULL);
}

void LedDriver_Flash(uint8_t ledIndex, uint16_t times) {
    if (ledIndex >= ledCount || leds[ledIndex].port == NULL) return;

    LedParams_t* params = pvPortMalloc(sizeof(LedParams_t));
    if (params == NULL) return;

    params->index = ledIndex;
    params->times = times;
    params->on_time = 100;  // 10% duty cycle: 100ms ON
    params->off_time = 900; // 10% duty cycle: 900ms OFF

    xTaskCreate(LedControlTask, "LedFlash", 128, params, osPriorityHigh, NULL);
}
