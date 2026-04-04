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

static void dispatch_get_encoder(void) {}

static void dispatch_get_all_encoders(void)
{
	sendBuffer[0] = 0x05;
	memset(sendBuffer + 1, 0xBB, 16);
	UARTTransport_send(sendBuffer, 17);
}

static void dispatch_get_imu(void)
{
	sendBuffer[0] = 0x06;
	memset(sendBuffer + 1, 0xCC, 52);
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
