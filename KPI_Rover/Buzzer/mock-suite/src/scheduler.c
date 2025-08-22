#define SCHEDULER_EMULATE_TIMERS 1

#include "scheduler.h"

#if SCHEDULER_EMULATE_TIMERS == 1
	#include "timers.h"
#endif /* SCHEDULER_EMULATE_TIMERS == 1 */

void scheduler_run_for(uint32_t ticks)
{
	for (int i = 0; i < ticks; i++)
	{
#if SCHEDULER_EMULATE_TIMERS == 1
		timers_run();
#endif /* SCHEDULER_EMULATE_TIMERS == 1 */
	}
}
