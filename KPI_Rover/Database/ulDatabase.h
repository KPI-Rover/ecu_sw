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
	PARAM_MOTOR_P_GAIN,
	PARAM_HEADING_OFFSET,
	PARAM_BATTERY_LEVEL,
	PARAM_ENCODER_COUNT,
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
struct ulDatabase_ParamMetadata *ulDatabase_getMetadata(uint16_t id);
bool ulDatabase_validateId(uint16_t id);
