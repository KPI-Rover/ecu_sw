#ifndef __FREERTOS_H
#define __FREERTOS_H


#include <stdint.h>

typedef int32_t TickType_t;

#define pdFALSE 0
#define pdTRUE 1

#define pdFAIL (pdFALSE)
#define pdPASS (pdTRUE)

#define pdMS_TO_TICKS(ms) (TickType_t) ms


#endif /* __FREERTOS_H */
