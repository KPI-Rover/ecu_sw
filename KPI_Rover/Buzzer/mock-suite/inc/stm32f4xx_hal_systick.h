#ifndef __STM32F4XX_HAL_SYSTICK_H
#define __STM32F4XX_HAL_SYSTICK_H

#include <stdint.h>

extern uint32_t system_ticks;

uint32_t HAL_GetTick(void);


#endif /* __STM32F4XX_HAL_SYSTICK_H */
