#include "ProtocolHandler.h"
#include "cmsis_os2.h"
#include "UARTTransport.h"
#include "messageQueueId.h"
#include <stdlib.h>
#include <string.h>

static const osThreadAttr_t ProtocolHandler_attributes = { .name =
		"ProtocolHandler_run", .stack_size = 128 * 4, .priority =
		(osPriority_t) osPriorityNormal, };

void ProtocolHandler_init(void)	{
	osThreadNew(ProtocolHandler_run, NULL, &ProtocolHandler_attributes);
}

void ProtocolHandler_run() {
	uint8_t request_prt[17];
	uint8_t response_ptr[53];

	osStatus_t status;
	while (1) {
		status = osMessageQueueGet(requestQueue, request_prt, NULL, 0);
		if (status != osOK) {
			return;
		}

		ProtocolHandler_processRequest(request_prt, response_ptr);
		osMessageQueuePut(responseQueue, response_ptr, 0, 0);
	}
}

void ProtocolHandler_processRequest(uint8_t *request, uint8_t *response) {
	enum COMMAND_ID command_id = request[0];
	response[0] = command_id;

	uint8_t *payload;

	switch (command_id) {
	case GET_API_VERSION:
		GET_API_VERSION_Request *api_request = (GET_API_VERSION_Request*)payload;

		GET_API_VERSION_Response api_response = { .api_version = 1 };
		memcpy(response + 1, &api_response, sizeof(GET_API_VERSION_Response));
		break;
	case SET_MOTOR_SPEED:
		SET_MOTOR_SPEED_Request *set_motor_request = (SET_MOTOR_SPEED_Request*)payload;

		// do work

		SET_MOTOR_SPEED_Response set_motor_response = { .status = 0 };
		memcpy(response + 1, &set_motor_response, sizeof(SET_MOTOR_SPEED_Response));
		break;
	case SET_ALL_MOTORS_SPEED:
		SET_ALL_MOTORS_SPEED_Request *set_all_motors_request = (SET_ALL_MOTORS_SPEED_Request*)payload;

		// do work

		SET_ALL_MOTORS_SPEED_Response set_all_motors_response = { .status = 0 };
		memcpy(response + 1, &set_all_motors_response, sizeof(SET_ALL_MOTORS_SPEED_Response));
		break;
	case GET_ENCODER:
		GET_ENCODER_Request *get_encoder_request = (GET_ENCODER_Request*)payload;

		// do work

		GET_ENCODER_Response get_encoder_response = { .encoder_value = 100 };
		memcpy(response + 1, &get_encoder_response, sizeof(GET_ENCODER_Response));
		break;
	case GET_ALL_ENCODERS:
		// do work

		GET_ALL_ENCODERS_Response get_all_encoders_response = {
				.encoder_value_motor_1 = 1,
				.encoder_value_motor_2 = 2,
				.encoder_value_motor_3 = 3,
				.encoder_value_motor_4 = 4, };
		memcpy(response + 1, &get_all_encoders_response, sizeof(GET_ALL_ENCODERS_Response));
		break;
	case GET_IMU:
		// do work

		GET_IMU_Response get_imu_response = {
				.accel_x = 92.4046,
				.accel_y = 59.3909,
				.accel_z = 30.6394,
				.gyro_x = 57.8941,
				.gyro_y = 74.0133,
				.gyro_z = 78.6926,
				.mag_x = 43.637,
				.mag_y = 33.2195,
				.mag_z = 77.888,
				.quat_w = 10.0887,
				.quat_x = 78.5084,
				.quat_y = 83.5159,
				.quat_z = 76.1209
		};
		memcpy(response + 1, &get_imu_response, sizeof(GET_IMU_Response));
		break;
	default:
		break;
	}
}

