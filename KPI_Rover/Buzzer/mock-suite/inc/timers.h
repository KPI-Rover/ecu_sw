#include "FreeRTOS.h"

#include "config.h"

#define TIMER_CREATED ( (uint8_t) 0x1 )
#define TIMER_ACTIVE ( (uint8_t) 0x2 )


typedef uint32_t UBaseType_t;
typedef int32_t BaseType_t;

typedef struct StaticTimer_t * TimerHandle_t;

typedef void (*TimerCallbackFunction_t)(TimerHandle_t xTimer);

struct StaticTimer_t {
	uint8_t status;
	TickType_t timer_ticks_left;

	void * pvTimerID;
	UBaseType_t uxAutoReload;
	TickType_t xTimerPeriodInTicks;
	TimerCallbackFunction_t pxCallbackFunction;
};

typedef struct StaticTimer_t StaticTimer_t;


void timers_init(void);
void timers_run(void);
uint32_t timers_check_health(void);
TimerHandle_t *get_timer_queue(void);

TimerHandle_t xTimerCreateStatic(
		const char * const pcTimerName,
		const TickType_t xTimerPeriodInTicks,
		const UBaseType_t uxAutoReload,
		void * const pvTimerID,
		TimerCallbackFunction_t pxCallbackFunction,
		StaticTimer_t *pxTimerBuffer
);

BaseType_t xTimerStop(
		TimerHandle_t xTimer,
		TickType_t xTicksToWait
);

BaseType_t xTimerChangePeriod(
		TimerHandle_t xTimer,
		TickType_t xNewPeriod,
		TickType_t xTicksToWait
);

BaseType_t xTimerStart(
		TimerHandle_t xTimer,
		TickType_t xTicksToWait
);
