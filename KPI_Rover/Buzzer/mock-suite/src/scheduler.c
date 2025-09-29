#define SCHEDULER_EMULATE_TIMERS 1

#include <stddef.h>
#include <stdint.h>

uint32_t system_ticks = 0;

#include "scheduler.h"

#if SCHEDULER_EMULATE_TIMERS == 1
	#include "timers.h"
#endif /* SCHEDULER_EMULATE_TIMERS == 1 */

void scheduler_run_for(uint32_t ticks)
{
	for (int i = 0; i < ticks; i++)
	{
		system_ticks++;

#if SCHEDULER_EMULATE_TIMERS == 1
		timers_run();
#endif /* SCHEDULER_EMULATE_TIMERS == 1 */
	}
}
