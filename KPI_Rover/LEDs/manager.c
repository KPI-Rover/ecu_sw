#include "manager.h"
#include "driver.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include <cmsis_os2.h>

#define NUM_LEDS 4

static osTimerId_t ledTimerHandle;

static LedSettings_t leds[] = {
	    {GPIOD, GPIO_PIN_12, LED_ON, 1},
	    {GPIOD, GPIO_PIN_13, LED_OFF, 1},
	    {GPIOD, GPIO_PIN_14, LED_BLINK, 4},
	    {GPIOD, GPIO_PIN_15, LED_FLASH, 3}
	};

static void LedTimer_Callback(void *argument) {
    LedDriver_TimerTask();
}

void LedManager_Init() {

    LedDriver_Init(leds, NUM_LEDS);

    const osTimerAttr_t ledTimer_attributes = {
        .name = "LedTimer",
    };

    ledTimerHandle = osTimerNew(LedTimer_Callback, osTimerPeriodic, NULL, &ledTimer_attributes);

    if (ledTimerHandle != NULL) {
        osTimerStart(ledTimerHandle, 100);
    }

}

void LedManager_Task(void *argument) {

	LedManager_Init();

	for (;;) {

		LedDriver_Flash(1, 4);
		LedDriver_Flash(3, 4);
		LedDriver_Blink(0, 4);
		LedDriver_Blink(2, 4);

		osDelay(6000);

		LedDriver_On(2);

		osDelay(500);

		LedDriver_Off(2);

		osDelay(500);

		for (uint8_t i = 0; i < NUM_LEDS; i++) {
			switch (leds[i].mode) {
				case LED_ON:
					LedDriver_On(i);
					break;
				case LED_OFF:
					LedDriver_Off(i);
					break;
				case LED_BLINK:
					LedDriver_Blink(i, leds[i].times);
					break;
				case LED_FLASH:
					LedDriver_Flash(i, leds[i].times);
					break;
				default:
					break;
			}
		}

		osDelay(5000);

		LedDriver_Off(0);

		osDelay(500);

	}

}
