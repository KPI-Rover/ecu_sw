#include "FreeRTOS.h"
#include "task.h"

#include "cmsis_os2.h"

#include "ul_ulog.h"

#include "driver.h"

#define LEN(x) ( sizeof(x) / sizeof(x[0]) )

static osTimerId_t timer_handle;
static StaticTimer_t timer;
static struct BuzzerObject bo[1];

static void buzzer_timer_callback(void *d)
{
	for (uint32_t i = 0; i < LEN(bo); i++)
		Buzzer_TimerTask(&(bo[i]));
}

void buzzer_manager_task(void *d)
{
	if (Buzzer_ConfigurePort(&(bo[0]), GPIOD, GPIO_PIN_15))
	{
		ULOG_ERROR("Failed to configure buzzer driver port");
		vTaskDelete(NULL);
	}

	{
		osTimerAttr_t timer_attrs = {
			.name = NULL,
			.attr_bits = 0,
			.cb_mem = &timer,
			.cb_size = sizeof(timer)
		};

		timer_handle = osTimerNew(buzzer_timer_callback, osTimerPeriodic, (void *) 0, &timer_attrs);

		if (timer_handle != (&timer)) {
			ULOG_ERROR("Failed to create a timer");
			vTaskDelete(NULL);
		}

		if (osTimerStart(timer_handle, 10) != osOK) {
			ULOG_ERROR("Failed to start a timer");
			vTaskDelete(NULL);
		}
	}

	for ( ; ; )
	{
		Buzzer_Enable(&(bo[0]));
		vTaskDelay(pdMS_TO_TICKS(3000));
		Buzzer_Disable(&(bo[0]));
		vTaskDelay(pdMS_TO_TICKS(3000));
		Buzzer_Pulse(&(bo[0]), 500, 1000, 5000);
		vTaskDelay(pdMS_TO_TICKS(5000));
		Buzzer_Pulse(&(bo[0]), 200, 1000, 5000);
		vTaskDelay(pdMS_TO_TICKS(5000));
		Buzzer_Pulse(&(bo[0]), 100, 300, 5000);
		vTaskDelay(pdMS_TO_TICKS(5000));
		Buzzer_Pulse(&(bo[0]), 250, 300, 5000);
		vTaskDelay(pdMS_TO_TICKS(1000)); // intentionally shorter delay; next command must abort this one
	}
}
