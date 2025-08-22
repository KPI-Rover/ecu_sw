#include "FreeRTOS.h"

typedef uint32_t UBaseType_t;
typedef int32_t BaseType_t;

typedef struct StaticTimer_t * TimerHandle_t;

typedef void (*TimerCallbackFunction_t)(TimerHandle_t xTimer);

struct StaticTimer_t {
	uint8_t status;
	TickType_t timer_ticks_left;

	UBaseType_t uxAutoReload;
	TickType_t xTimerPeriodInTicks;
	TimerCallbackFunction_t pxCallbackFunction;
};

typedef struct StaticTimer_t StaticTimer_t;


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
