#ifndef __CMSIS_OS2_H
#define __CMSIS_OS2_H

#include <stddef.h>
#include "timers.h"

typedef StaticTimer_t * osTimerId_t;

typedef enum osStatus_t {
	osOK,
	osError,
	osErrorTimeout,
	osErrorResource,
	osErrorParameter,
	osErrorNoMemory,
	osErrorISR,
	osStatusReserved = 0x7FFFFFFF
} osStatus_t;

typedef struct {
	char *name;
	uint32_t attr_bits;
	TimerHandle_t cb_mem;
	size_t cb_size;
} osTimerAttr_t;

osTimerId_t osTimerNew(TimerCallbackFunction_t cb, uint32_t repeat, void * arg, osTimerAttr_t * attrs);
osStatus_t osTimerStart(osTimerId_t id, uint32_t interval);


#endif /* __CMSIS_OS2_H */
