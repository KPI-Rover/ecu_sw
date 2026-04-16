#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <stdlib.h>

#include "cmsis_os2.h"

enum ulDatabase_ParamType {
	UINT8,
	INT8,
	UINT16,
	INT16,
	UINT32,
	INT32,
	FLOAT
};

enum ulDatabase_ParamId {
	ENCODER_CONTROL_PERIOD_MS,
	ENCODER_TICKS_PER_REVOLUTION,
	MOTOR_FL_KP,
	MOTOR_FL_KI,
	MOTOR_FL_KD,
	MOTOR_FR_KP,
	MOTOR_FR_KI,
	MOTOR_FR_KD,
	MOTOR_RL_KP,
	MOTOR_RL_KI,
	MOTOR_RL_KD,
	MOTOR_RR_KP,
	MOTOR_RR_KI,
	MOTOR_RR_KD,
	MOTOR_FL_SETPOINT,
	MOTOR_FR_SETPOINT,
	MOTOR_RL_SETPOINT,
	MOTOR_RR_SETPOINT,
	MOTOR_FL_RPM,
	MOTOR_FR_RPM,
	MOTOR_RL_RPM,
	MOTOR_RR_RPM,
	IMU_ACCEL_X,
	IMU_ACCEL_Y,
	IMU_ACCEL_Z,
	IMU_GYRO_X,
	IMU_GYRO_Y,
	IMU_GYRO_Z,
	IMU_MAG_X,
	IMU_MAG_Y,
	IMU_MAG_Z,
	IMU_QUAT_W,
	IMU_QUAT_X,
	IMU_QUAT_Y,
	IMU_QUAT_Z,
	IMU_CALIB_CMD,
	IMU_CALIB_STATUS,
	IMU_IS_CALIBRATED,
	ADC_CALIBRATION_START,
	ADC_CALIBRATION_POINT,
	ADC_CALIBRATION_POINT_VALUE,
	ADC_CAL_CH_11_OFFSET,
	ADC_CAL_CH_TEMP_OFFSET,
	ADC_CAL_CH_11_SCALE,
	ADC_CAL_CH_TEMP_SCALE,
	ADC_CALIBRATION_CHANNEL_ID,
	PARAM_BATTERY_VOLTAGE,
	PARAM_MCU_TEMPERATURE,
	PARAM_COUNT
};

struct ulDatabase_ParamMetadata {
	uint16_t offset;
	enum ulDatabase_ParamType type;
	bool persistent;
	float defaultValue;
};

struct ulDatabase {
	uint8_t *dataArray;
	struct ulDatabase_ParamMetadata *metadataTable;
	uint16_t metadataCount;
	uint16_t dataArraySize;
};

bool ulDatabase_init(struct ulDatabase_ParamMetadata * metadataTable, uint16_t metadataCount);
bool ulDatabase_setUint8(uint16_t id, uint8_t value);
bool ulDatabase_getUint8(uint16_t id, uint8_t *value);
bool ulDatabase_setInt8(uint16_t id, int8_t value);
bool ulDatabase_getInt8(uint16_t id, int8_t *value);
bool ulDatabase_setUint16(uint16_t id, uint16_t value);
bool ulDatabase_getUint16(uint16_t id, uint16_t *value);
bool ulDatabase_setInt16(uint16_t id, int16_t value);
bool ulDatabase_getInt16(uint16_t id, int16_t *value);
bool ulDatabase_setUint32(uint16_t id, uint32_t value);
bool ulDatabase_getUint32(uint16_t id, uint32_t *value);
bool ulDatabase_setInt32(uint16_t id, int32_t value);
bool ulDatabase_getInt32(uint16_t id, int32_t *value);
bool ulDatabase_setFloat(uint16_t id, float value);
bool ulDatabase_getFloat(uint16_t id, float *value);
bool ulDatabase_reset(uint16_t id);
uint8_t *ulDatabase_freeze(void);
void ulDatabase_unfreeze(void);
struct ulDatabase_ParamMetadata *ulDatabase_getMetadata(uint16_t id);
bool ulDatabase_validateId(uint16_t id);
