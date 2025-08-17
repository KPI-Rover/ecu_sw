#include "FreeRTOS.h"
#include "task.h"

#include "ul_ulog.h"

#include "driver.h"

void buzzer_manager_task(void *d)
{
	if (Buzzer_ConfigurePort(GPIOD, GPIO_PIN_15))
	{
		ULOG_ERROR("Failed to configure buzzer driver port");
		vTaskDelete(NULL);
	}

	for ( ; ; )
	{
		Buzzer_Enable();
		vTaskDelay(pdMS_TO_TICKS(3000));
		Buzzer_Disable();
		vTaskDelay(pdMS_TO_TICKS(3000));
		Buzzer_Pulse(500, 1000, 5000);
		vTaskDelay(pdMS_TO_TICKS(5000));
		Buzzer_Pulse(200, 1000, 5000);
		vTaskDelay(pdMS_TO_TICKS(5000));
		Buzzer_Pulse(100, 300, 5000);
		vTaskDelay(pdMS_TO_TICKS(5000));
		Buzzer_Pulse(250, 300, 5000);
		vTaskDelay(pdMS_TO_TICKS(1000)); // intentionally shorter delay; next command must abort this one
	}
}
