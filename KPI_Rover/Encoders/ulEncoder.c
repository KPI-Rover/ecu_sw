#include "ulEncoder.h"
#include "drvEncoder.h"

#include "FreeRTOS.h"
#include "cmsis_os2.h"


#define TICKS_PER_REVOLUTION 820.0f
#define MOTORS_CONTROL_PERIOD_MS 20
#define MOTORS_COUNT 4


static uint32_t lastTicks_RPM[MOTORS_COUNT];
static uint32_t lastTicks_Ethernet[MOTORS_COUNT];

static osTimerId_t encoderTimerHandle;


static void ulEncoder_TimerCallback(void *argument) {

	for (int i = 0; i < MOTORS_COUNT; i++) {
        uint32_t currentTicks = EncoderDriver_Read(i);
        uint32_t lastTicks = lastTicks_RPM[i];

        int32_t diff = (int32_t) (currentTicks - lastTicks);

        float ticks_per_sec = (float) diff * (1000.0f / MOTORS_CONTROL_PERIOD_MS);
        float rpm = ticks_per_sec * 60.0f/ TICKS_PER_REVOLUTION;

        // TODO: adding rpm to the database

        lastTicks_RPM[i] = currentTicks;
    }
}

void ulEncoder_Init(void) {

	EncoderDriver_Init();

    for (int i = 0; i < MOTORS_COUNT; i++) {
        uint32_t initialTicks = EncoderDriver_Read(i);
        lastTicks_RPM[i] = initialTicks;
        lastTicks_Ethernet[i] = initialTicks;
    }

    const osTimerAttr_t encoderTimer_attributes = {
      .name = "EncoderTimer",
    };

    encoderTimerHandle = osTimerNew(ulEncoder_TimerCallback, osTimerPeriodic, NULL, &encoderTimer_attributes);

    if (encoderTimerHandle != NULL) {
      osTimerStart(encoderTimerHandle, MOTORS_CONTROL_PERIOD_MS);
    }
}

void ulEncoder_GetDiffForEthernet(int32_t *diffOutput) {

    for (int i = 0; i < MOTORS_COUNT; i++) {

        uint32_t currentTicks = EncoderDriver_Read(i);
        uint32_t lastTicks = lastTicks_Ethernet[i];

        int32_t diff = (int32_t)(currentTicks - lastTicks);

        diffOutput[i] = diff;
        lastTicks_Ethernet[i] = currentTicks;
    }
}
