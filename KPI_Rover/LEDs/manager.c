#include "manager.h"
#include "driver.h"

#define NUM_LEDS 7

void LedManager_Init() {

	LedSettings_t leds[] = {
	    {GPIOD, GPIO_PIN_12, LED_ON, 1},
	    {GPIOD, GPIO_PIN_13, LED_ON, 1},
	    {GPIOD, GPIO_PIN_14, LED_ON, 1},
	    {GPIOD, GPIO_PIN_15, LED_ON, 1}
	};

    LedDriver_Init(leds, NUM_LEDS);
}

void LedManager_Task(void *argument) {

	LedManager_Init();

	for (;;) {

	}

}
