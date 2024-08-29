#include <stddef.h>
#include "heartbeat.h"
#include "cmsis_os2.h"
#include "freertos.h"
#include "timers.h"


#define MS_TO_TICKS(ms) ((ms) / portTICK_PERIOD_MS)

void HeartbeatLedOn(void *arg);
void HeartbeatLedOff(void *arg);

static osTimerId_t ledOnTimer = NULL;
static osTimerId_t ledOffTimer = NULL;

int HeartbeatInit(void){
	/*
	 * Creates and starts software timers for LED blinking.
	 * returns 1 in case of error of timer creation or starting
	 *         0 in case of success
	 */
	ledOnTimer = osTimerNew(HeartbeatLedOn, osTimerPeriodic, NULL, NULL);
	ledOffTimer = osTimerNew(HeartbeatLedOff, osTimerPeriodic, NULL, NULL);

	if (ledOnTimer != NULL && ledOffTimer != NULL){
		if (!osTimerStart( ledOnTimer, MS_TO_TICKS(normal) ) && xTimerPendFunctionCall( (PendedFunction_t) osTimerStart,
				(void *) ledOffTimer, MS_TO_TICKS(normal), MS_TO_TICKS(LED_FLASH_DURATION_MS)) != pdFALSE){

				return 0;
			}

	}
	return 1;
}

int HeartbeatModeSwitch(heartbeatMode mode){
	/*
	 * Switches heartbeat LED blinking mode.
	 * Important! Must be called after the scheduler was started i.e. inside a task
	 *
	 * returns 1 in case of error of changing timers' periods
	 *         0 in case of success
	 */
	BaseType_t status1 = xTimerChangePeriod( ledOnTimer, MS_TO_TICKS(mode), 5);
	osDelay(MS_TO_TICKS(LED_FLASH_DURATION_MS));
	BaseType_t status2 = xTimerChangePeriod( ledOffTimer, MS_TO_TICKS(mode), 5);
	if (status1 == pdPASS && status2 == pdPASS){
		return 0;
	}
	return 1;
}

void HeartbeatLedOn(void *arg){
	HAL_GPIO_WritePin(HEARTBEAT_LED_Port, HEARTBEAT_LED_Pin, GPIO_PIN_SET);
}

void HeartbeatLedOff(void *arg){
	HAL_GPIO_WritePin(HEARTBEAT_LED_Port, HEARTBEAT_LED_Pin, GPIO_PIN_RESET);
}


