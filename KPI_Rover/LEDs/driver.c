#include "driver.h"
#include "cmsis_os.h"
#include <stdlib.h>

#define MAX_LEDS 7

static LedSettings_t ledsInitialized[MAX_LEDS];
static uint8_t ledCount = 0;

void LedDriver_Init(LedSettings_t *self, uint8_t count) {
	if (self == NULL || count == 0) return;

    if (count > MAX_LEDS) count = MAX_LEDS;
    ledCount = count;

    for (uint8_t i = 0; i < count; i++) {
    	if (self[i].port == NULL) continue;
    	ledsInitialized[i] = self[i];
        HAL_GPIO_WritePin(ledsInitialized[i].port, ledsInitialized[i].pin, GPIO_PIN_RESET);
    }
}

void LedDriver_On(uint8_t ledIndex) {
    if (ledsInitialized[ledIndex].port == NULL) return;
    if (ledIndex < ledCount)
        HAL_GPIO_WritePin(ledsInitialized[ledIndex].port, ledsInitialized[ledIndex].pin, GPIO_PIN_SET);
}

void LedDriver_Off(uint8_t ledIndex) {
    if (ledsInitialized[ledIndex].port == NULL) return;
    if (ledIndex < ledCount)
        HAL_GPIO_WritePin(ledsInitialized[ledIndex].port, ledsInitialized[ledIndex].pin, GPIO_PIN_RESET);
}
