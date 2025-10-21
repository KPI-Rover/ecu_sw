#include <stdio.h>
#include <string.h>

#include "timers.h"

#include "fail.h"


static uint32_t null_timer_ptr_count = 0;
static uint32_t static_timer_recreation_count = 0;
static uint32_t non_positive_timer_period_count = 0;
static uint32_t null_callback_function_specified_count = 0;
static uint32_t operated_on_active_timer_count = 0;
static uint32_t timer_queue_overflow_count = 0;

static TimerHandle_t timer_queue[TIMER_QUEUE_SIZE];

static int register_timer(TimerHandle_t t)
{
	TimerHandle_t *vacant_timer = NULL;

	for (int i = 0; i < TIMER_QUEUE_SIZE; i++) {
		if (timer_queue[i] == t)
			return 0;

		if (timer_queue[i])
			continue;

		if (!vacant_timer)
			vacant_timer = &(timer_queue[i]);
	}

	if (vacant_timer) {
		*vacant_timer = t;
		return 0;
	} else {
		return -1;
	}
}

static int unregister_timer(TimerHandle_t t)
{
	for (int i = 0; i < TIMER_QUEUE_SIZE; i++) {
		if (timer_queue[i] != t)
			continue;

		timer_queue[i] = NULL;
		return i;
	}

	return -1;
}

void timers_init(void)
{
	memset(timer_queue, 0, sizeof(timer_queue));
}

uint32_t timers_check_health(void)
{
	uint32_t issues = 0;

	REPORT_COUNTER(null_timer_ptr_count);
	REPORT_COUNTER(static_timer_recreation_count);
	REPORT_COUNTER(non_positive_timer_period_count);
	REPORT_COUNTER(null_callback_function_specified_count);
	REPORT_COUNTER(operated_on_active_timer_count);
	REPORT_COUNTER(timer_queue_overflow_count);

	return issues;
}

void timers_run(void)
{
	TimerHandle_t chosen_timer = NULL;

	for (int i = 0; i < TIMER_QUEUE_SIZE; i++) {
		if (timer_queue[i] == NULL)
			continue;

		if (timer_queue[i]->status & TIMER_ACTIVE)
			timer_queue[i]->timer_ticks_left--;

		if (timer_queue[i]->timer_ticks_left > 0)
			continue;

		timer_queue[i]->status &= ~TIMER_ACTIVE;

		if (chosen_timer == NULL) {
			chosen_timer = timer_queue[i];
			timer_queue[i] = NULL;

			if (chosen_timer->uxAutoReload)
				xTimerStart(chosen_timer, chosen_timer->xTimerPeriodInTicks);
		}
	}

	if (chosen_timer)
		chosen_timer->pxCallbackFunction(chosen_timer);
}

TimerHandle_t *get_timer_queue(void)
{
	return timer_queue;
}

TimerHandle_t xTimerCreateStatic(
		const char * const pcTimerName,
		const TickType_t xTimerPeriodInTicks,
		const UBaseType_t uxAutoReload,
		void * const pvTimerID,
		TimerCallbackFunction_t pxCallbackFunction,
		StaticTimer_t *pxTimerBuffer)
{
	FAIL_ON_EQ(pxTimerBuffer, NULL, NULL, "WARN", null_timer_ptr_count);

	FAIL_ON_EQ(pxTimerBuffer->status & TIMER_CREATED, TIMER_CREATED, pxTimerBuffer, "CRIT", static_timer_recreation_count);
	pxTimerBuffer->status = TIMER_CREATED;

	pxTimerBuffer->xTimerPeriodInTicks = xTimerPeriodInTicks;

	pxTimerBuffer->uxAutoReload = uxAutoReload;

	FAIL_ON_EQ(pxCallbackFunction, NULL, NULL, "WARN", null_callback_function_specified_count);
	pxTimerBuffer->pxCallbackFunction = pxCallbackFunction;

	pxTimerBuffer->pvTimerID = pvTimerID;

	return pxTimerBuffer;
}

BaseType_t xTimerStop(
		TimerHandle_t xTimer,
		TickType_t xTicksToWait)
{
	FAIL_ON_EQ(xTimer, NULL, pdFAIL, "WARN", null_timer_ptr_count);
	xTimer->status &= ~TIMER_ACTIVE;

	(void) unregister_timer(xTimer);

	return pdPASS;
}

BaseType_t xTimerChangePeriod(
		TimerHandle_t xTimer,
		TickType_t xNewPeriod,
		TickType_t xTicksToWait)
{
	FAIL_ON_EQ(xTimer, NULL, pdFAIL, "WARN", null_timer_ptr_count);
	WARN_ON_EQ(xTimer->status & TIMER_ACTIVE, TIMER_ACTIVE, "WARN", operated_on_active_timer_count);

	FAIL_ON_LE(xNewPeriod, 0, pdFAIL, "ERROR", non_positive_timer_period_count);

	xTimer->xTimerPeriodInTicks = xNewPeriod;

	return pdPASS;
}

BaseType_t xTimerStart(
		TimerHandle_t xTimer,
		TickType_t xTicksToWait)
{
	FAIL_ON_EQ(xTimer, NULL, pdFAIL, "WARN", null_timer_ptr_count);
	xTimer->status |= TIMER_ACTIVE;
	xTimer->timer_ticks_left = xTimer->xTimerPeriodInTicks;

	FAIL_ON_EQ(register_timer(xTimer), -1, pdFAIL, "WARN", timer_queue_overflow_count);

	return pdPASS;
}
