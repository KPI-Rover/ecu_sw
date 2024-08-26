/*
 * heartbeat.h
 *
 *  Created on: Aug 23, 2024
 *      Author: Victoria
 */

#ifndef INC_HEARTBEAT_H_
#define INC_HEARTBEAT_H_

#include "main.h"

#define HEARTBEAT_LED_Pin LED_RED_Pin
#define HEARTBEAT_LED_Port LED_RED_GPIO_Port

#define LED_FLASH_DURATION_MS 100


typedef enum {
	normal = 1000,
	error = 3000
} heartbeatMode;		//numbers are periods of LED blinking in milliseconds


int HeartbeatInit(void);
int HeartbeatModeSwitch(heartbeatMode mode);

#endif /* INC_HEARTBEAT_H_ */
