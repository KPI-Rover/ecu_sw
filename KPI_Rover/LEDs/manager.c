#include "manager.h"
#include "driver.h"

#define NUM_LEDS 4

typedef enum {
	LED_ON,
	LED_OFF,
	LED_BLINK,
	LED_FLASH
} LedMode_t;

typedef struct {
	LedMode_t mode;
    uint16_t times;
} LedSettings_t;

static LedSettings_t settings[NUM_LEDS];

void LedManager_Init() {

	Led_t leds[] = {
	    {GPIOD, GPIO_PIN_12},
	    {GPIOD, GPIO_PIN_13},
	    {GPIOD, GPIO_PIN_14},
	    {GPIOD, GPIO_PIN_15}
	};

    LedDriver_Init(leds, NUM_LEDS);

    settings[0] = (LedSettings_t){LED_ON, 1};
    settings[1] = (LedSettings_t){LED_OFF, 1};
    settings[2] = (LedSettings_t){LED_BLINK, 5};
    settings[3] = (LedSettings_t){LED_FLASH, 7};
}

void LedManager_Task(void *argument) {

	LedManager_Init();

	for (;;) {
		for (uint8_t i = 0; i < NUM_LEDS; i++) {
			switch (settings[i].mode) {
				case LED_ON:
					LedDriver_On(i);
					break;
				case LED_OFF:
					LedDriver_Off(i);
					break;
				case LED_BLINK:
					LedDriver_Blink(i, settings[i].times);
					break;
				case LED_FLASH:
					LedDriver_Flash(i, settings[i].times);
					break;
			}
		}
	}

}
