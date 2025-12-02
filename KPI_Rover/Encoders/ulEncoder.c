#include "ulEncoder.h"
#include "drvEncoder.h"
#include "Database/ulDatabase.h"

#include "FreeRTOS.h"
#include "cmsis_os2.h"


#define MOTORS_COUNT 4

#define TIMER_16_BIT_MAX 0x10000
#define OVERFLOW_THRESHOLD TIMER_16_BIT_MAX / 2


static uint32_t lastTicks_RPM[MOTORS_COUNT];
static uint32_t lastTicks_Ethernet[MOTORS_COUNT];

static uint16_t encoder_period_ms = 5;
static float encoder_ticks_per_rev = 820.0f;

static osTimerId_t encoderTimerHandle;


static inline int32_t ulEncoder_CalculateDiff(uint32_t current, uint32_t last) {
    int32_t diff = (int32_t)(current - last);

    // overflow in 16-bit timer
    if (diff < -OVERFLOW_THRESHOLD) {
        diff += TIMER_16_BIT_MAX;
    }
    else if (diff > OVERFLOW_THRESHOLD) {
        diff -= TIMER_16_BIT_MAX;
    }

    return diff;
}

static void ulEncoder_TimerCallback(void *argument) {

	for (int i = 0; i < MOTORS_COUNT; i++) {
        uint32_t currentTicks = drvEncoder_Read(i);
        uint32_t lastTicks = lastTicks_RPM[i];

        int32_t diff = ulEncoder_CalculateDiff(currentTicks, lastTicks);

        float ticks_per_sec = (float) diff * (1000.0f / encoder_period_ms);
        float rpm = ticks_per_sec * 60.0f / encoder_ticks_per_rev;

        int32_t rpm_to_db = (int32_t)rpm;
        ulDatabase_setInt32(MOTOR_FL_RPM + i, rpm_to_db);

        lastTicks_RPM[i] = currentTicks;
    }
}

void ulEncoder_Init(void) {

	drvEncoder_Init();

	if (!ulDatabase_getUint16(ENCODER_CONTROL_PERIOD_MS, &encoder_period_ms) || encoder_period_ms == 0) {
		encoder_period_ms = 5;
	}

	if (!ulDatabase_getFloat(ENCODER_TICKS_PER_REVOLUTION, &encoder_ticks_per_rev) || encoder_ticks_per_rev < 1.0f) {
	        encoder_ticks_per_rev = 820.0f;
	}

    for (int i = 0; i < MOTORS_COUNT; i++) {
        uint32_t initialTicks = drvEncoder_Read(i);
        lastTicks_RPM[i] = initialTicks;
        lastTicks_Ethernet[i] = initialTicks;
    }

    const osTimerAttr_t encoderTimer_attributes = {
      .name = "EncoderTimer",
    };

    encoderTimerHandle = osTimerNew(ulEncoder_TimerCallback, osTimerPeriodic, NULL, &encoderTimer_attributes);

    if (encoderTimerHandle != NULL) {
      osTimerStart(encoderTimerHandle, encoder_period_ms);
    }
}

void ulEncoder_GetDiffForEthernet(int32_t *diffOutput) {

    for (int i = 0; i < MOTORS_COUNT; i++) {

        uint32_t currentTicks = drvEncoder_Read(i);
        uint32_t lastTicks = lastTicks_Ethernet[i];

        int32_t diff = ulEncoder_CalculateDiff(currentTicks, lastTicks);

        diffOutput[i] = diff;
        lastTicks_Ethernet[i] = currentTicks;
    }
}

