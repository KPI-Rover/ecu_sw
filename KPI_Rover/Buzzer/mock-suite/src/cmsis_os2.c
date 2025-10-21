#include "cmsis_os2.h"

osTimerId_t osTimerNew(TimerCallbackFunction_t cb, uint32_t repeat, void * arg, osTimerAttr_t * attrs)
{
	return xTimerCreateStatic(NULL, 0, repeat, arg, cb, attrs->cb_mem);
}

osStatus_t osTimerStart(osTimerId_t id, uint32_t interval)
{
	id->xTimerPeriodInTicks = interval;
	return xTimerStart(id, 0);
}
