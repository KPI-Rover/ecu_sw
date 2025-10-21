#include "stm32f4xx_hal.h"

#include "generic.h"

uint32_t ticks_elapsed(const uint32_t old_ticks, const uint32_t new_ticks)
{
	return (uint32_t) (((uint32_t) new_ticks) - ((uint32_t) old_ticks));
}

uint32_t ticks_elapsed_since(const uint32_t old_ticks)
{
	int32_t current_ticks = HAL_GetTick();
	return (uint32_t) (((uint32_t) current_ticks) - ((uint32_t) old_ticks));
}
