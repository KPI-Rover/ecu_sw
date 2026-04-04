#include "UARTTransport.h"

#include <string.h>
#include <stdint.h>
#include "cmsis_os2.h"

#include "Database/ulDatabase.h"

#define SEND_BUFFER_SIZE 64
#define RECV_BUFFER_SIZE 32

#define LEN(x) ( sizeof(x) / sizeof((x)[0]) )

static uint8_t sendBuffer[SEND_BUFFER_SIZE];
static uint8_t recvBuffer[RECV_BUFFER_SIZE];
static uint8_t recvSize;

static osThreadAttr_t ta = {
	.stack_size = 64 * 4
};

static int32_t switch_endianness_int32(const uint8_t * const address)
{
	return (int32_t) ((((uint32_t)address[0]) << 24) + (((uint32_t)address[1]) << 16) + (((uint32_t)address[2]) << 8) + ((uint32_t)address[3]));
}

static void dispatch_not_a_command(void) {}

static void dispatch_get_api_version(void)
{
	sendBuffer[0] = 0x01;
	sendBuffer[1] = 0x01;
	UARTTransport_send(sendBuffer, 2);
}

static void dispatch_set_motor_speed(void)
{
	sendBuffer[0] = 0x02;
	sendBuffer[1] = 0x00;

	static int32_t target_value;

	target_value = switch_endianness_int32(&(recvBuffer[2])); // big (network) to little (host)

	switch (recvBuffer[1]) {
	case 0:
		ulDatabase_setInt32(TARGET_MOTOR_FL_RPM, target_value);
		break;
	case 1:
		ulDatabase_setInt32(TARGET_MOTOR_RL_RPM, target_value);
		break;
	case 2:
		ulDatabase_setInt32(TARGET_MOTOR_FR_RPM, target_value);
		break;
	case 3:
		ulDatabase_setInt32(TARGET_MOTOR_RR_RPM, target_value);
		break;
	default:
		sendBuffer[1] = 0x01;
		break;
	}

	UARTTransport_send(sendBuffer, 2);
}

static void dispatch_set_all_motors_speed(void)
{
	static int32_t target_value_fl, target_value_rl, target_value_fr, target_value_rr;

	target_value_fl = switch_endianness_int32(&(recvBuffer[1]));
	target_value_rl = switch_endianness_int32(&(recvBuffer[5]));
	target_value_fr = switch_endianness_int32(&(recvBuffer[9]));
	target_value_rr = switch_endianness_int32(&(recvBuffer[13]));

	ulDatabase_setInt32(TARGET_MOTOR_FL_RPM, target_value_fl);
	ulDatabase_setInt32(TARGET_MOTOR_RL_RPM, target_value_rl);
	ulDatabase_setInt32(TARGET_MOTOR_FR_RPM, target_value_fr);
	ulDatabase_setInt32(TARGET_MOTOR_RR_RPM, target_value_rr);

	sendBuffer[0] = 0x03;
	sendBuffer[1] = 0x00;
	UARTTransport_send(sendBuffer, 2);
}

static void dispatch_get_encoder(void)
{
	sendBuffer[0] = 0x04;

	static int32_t source_value;

	switch (recvBuffer[1]) {
	case 0:
		ulDatabase_getInt32(ENCODER_MOTOR_FL_RPM, &source_value);
		break;
	case 1:
		ulDatabase_getInt32(ENCODER_MOTOR_RL_RPM, &source_value);
		break;
	case 2:
		ulDatabase_getInt32(ENCODER_MOTOR_FR_RPM, &source_value);
		break;
	case 3:
		ulDatabase_getInt32(ENCODER_MOTOR_RR_RPM, &source_value);
		break;
	default:
		// failure is not an option in the protocol
		break;
	}

	source_value = switch_endianness_int32((uint8_t *) &source_value); // little (host) to big (network)
	memcpy(&(sendBuffer[1]), &source_value, sizeof(source_value));

	UARTTransport_send(sendBuffer, 5);
}

static void dispatch_get_all_encoders(void)
{
	static int32_t source_value_fl, source_value_rl, source_value_fr, source_value_rr;

	ulDatabase_getInt32(ENCODER_MOTOR_FL_RPM, &source_value_fl);
	ulDatabase_getInt32(ENCODER_MOTOR_RL_RPM, &source_value_rl);
	ulDatabase_getInt32(ENCODER_MOTOR_FR_RPM, &source_value_fr);
	ulDatabase_getInt32(ENCODER_MOTOR_RR_RPM, &source_value_rr);

	source_value_fl = switch_endianness_int32((uint8_t *) &source_value_fl);
	source_value_rl = switch_endianness_int32((uint8_t *) &source_value_rl);
	source_value_fr = switch_endianness_int32((uint8_t *) &source_value_fr);
	source_value_rr = switch_endianness_int32((uint8_t *) &source_value_rr);

	sendBuffer[0] = 0x05;
	memcpy(&(sendBuffer[1]), &source_value_fl, sizeof(source_value_fl));
	memcpy(&(sendBuffer[5]), &source_value_rl, sizeof(source_value_rl));
	memcpy(&(sendBuffer[9]), &source_value_fr, sizeof(source_value_fr));
	memcpy(&(sendBuffer[13]), &source_value_rr, sizeof(source_value_rr));

	UARTTransport_send(sendBuffer, 17);
}

static void dispatch_get_imu(void)
{
	sendBuffer[0] = 0x06;

	ulDatabase_getFloat(IMU_ACCEL_X, (float *) (sendBuffer + 1));
	ulDatabase_getFloat(IMU_ACCEL_Y, (float *) (sendBuffer + 5));
	ulDatabase_getFloat(IMU_ACCEL_Z, (float *) (sendBuffer + 9));

	ulDatabase_getFloat(IMU_GYRO_X, (float *) (sendBuffer + 13));
	ulDatabase_getFloat(IMU_GYRO_Y, (float *) (sendBuffer + 17));
	ulDatabase_getFloat(IMU_GYRO_Z, (float *) (sendBuffer + 21));

	ulDatabase_getFloat(IMU_MAG_X, (float *) (sendBuffer + 25));
	ulDatabase_getFloat(IMU_MAG_Y, (float *) (sendBuffer + 29));
	ulDatabase_getFloat(IMU_MAG_Z, (float *) (sendBuffer + 33));

	ulDatabase_getFloat(IMU_QUAT_X, (float *) (sendBuffer + 37));
	ulDatabase_getFloat(IMU_QUAT_X, (float *) (sendBuffer + 41));
	ulDatabase_getFloat(IMU_QUAT_Y, (float *) (sendBuffer + 45));
	ulDatabase_getFloat(IMU_QUAT_Z, (float *) (sendBuffer + 49));

	UARTTransport_send(sendBuffer, 53);
}

static const void (*dispatch_table[])(void) = {
	dispatch_not_a_command,
	dispatch_get_api_version,
	dispatch_set_motor_speed,
	dispatch_set_all_motors_speed,
	dispatch_get_encoder,
	dispatch_get_all_encoders,
	dispatch_get_imu,
};

void ProtocolHandler_processTask(void *d)
{
	(void) d;

	UARTTransport_init();

	for ( ; ; ) {
		UARTTransport_receive(recvBuffer, &recvSize);

		if (recvBuffer[0] >= LEN(dispatch_table))
			continue;

		dispatch_table[recvBuffer[0]]();
	}
}

void ProtocolHandler_init(void)
{
	(void) osThreadNew(ProtocolHandler_processTask, NULL, &ta);
}
