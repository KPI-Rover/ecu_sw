#ifndef COMMUNICATION_PROTOCOLHANDLER_H_
#define COMMUNICATION_PROTOCOLHANDLER_H_

#include <stdint.h>

enum COMMAND_ID {
	GET_API_VERSION = 0x01,
	SET_MOTOR_SPEED = 0X02,
	SET_ALL_MOTORS_SPEED = 0X03,
	GET_ENCODER = 0X04,
	GET_ALL_ENCODERS = 0X05,
	GET_IMU = 0X06,
};

typedef struct {
	uint8_t driver_version;
} GET_API_VERSION_Request;

typedef struct __attribute__((__packed__)) {
	uint8_t motor_id;
	uint32_t speed;
} SET_MOTOR_SPEED_Request;

typedef struct {
	uint32_t speed_motor_1;
	uint32_t speed_motor_2;
	uint32_t speed_motor_3;
	uint32_t speed_motor_4;
} SET_ALL_MOTORS_SPEED_Request;

typedef struct {
	uint32_t motor_id;
} GET_ENCODER_Request;

typedef struct {
	uint8_t api_version;
} GET_API_VERSION_Response;

typedef struct {
	uint8_t status;
} SET_MOTOR_SPEED_Response;

typedef struct {
	uint8_t status;
} SET_ALL_MOTORS_SPEED_Response;

typedef struct {
	uint32_t encoder_value;
} GET_ENCODER_Response;

typedef struct {
	uint32_t encoder_value_motor_1;
	uint32_t encoder_value_motor_2;
	uint32_t encoder_value_motor_3;
	uint32_t encoder_value_motor_4;
} GET_ALL_ENCODERS_Response;

typedef struct {
	float accel_x;
	float accel_y;
	float accel_z;
	float gyro_x;
	float gyro_y;
	float gyro_z;
	float mag_x;
	float mag_y;
	float mag_z;
	float quat_w;
	float quat_x;
	float quat_y;
	float quat_z;
} GET_IMU_Response;

void ProtocolHandler_run();
void ProtocolHandler_processRequest(uint8_t *request, uint8_t *response);

#endif /* COMMUNICATION_PROTOCOLHANDLER_H_ */
