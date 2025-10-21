#include "stm32f4xx_hal_systick.h"

extern uint32_t system_ticks;

uint32_t HAL_GetTick(void)
{
	return system_ticks;
}
