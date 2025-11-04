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

void ulDatabase_ParamMetadata_getSize(struct ulDatabase_ParamMetadata * self, uint16_t id);

struct ulDatabase {
	uint8_t *dataArray;
	struct ulDatabase_ParamMetadata *metadataTable;
	uint16_t metadataCount;
	uint16_t dataArraySize;
};

bool ulDatabase_init(struct ulDatabase * self, struct ulDatabase_ParamMetadata * metadataTable, uint16_t metadataCount);
bool ulDatabase_setUint8(struct ulDatabase * self, uint16_t id, uint8_t value);
bool ulDatabase_getUint8(struct ulDatabase * self, uint16_t id, uint8_t *value);
bool ulDatabase_setInt8(struct ulDatabase * self, uint16_t id, int8_t value);
bool ulDatabase_getInt8(struct ulDatabase * self, uint16_t id, int8_t *value);
bool ulDatabase_setUint16(struct ulDatabase * self, uint16_t id, uint16_t value);
bool ulDatabase_getUint16(struct ulDatabase * self, uint16_t id, uint16_t *value);
bool ulDatabase_setInt16(struct ulDatabase * self, uint16_t id, int16_t value);
bool ulDatabase_getInt16(struct ulDatabase * self, uint16_t id, int16_t *value);
bool ulDatabase_setUint32(struct ulDatabase * self, uint16_t id, uint32_t value);
bool ulDatabase_getUint32(struct ulDatabase * self, uint16_t id, uint32_t *value);
bool ulDatabase_setInt32(struct ulDatabase * self, uint16_t id, int32_t value);
bool ulDatabase_getInt32(struct ulDatabase * self, uint16_t id, int32_t *value);
bool ulDatabase_setFloat(struct ulDatabase * self, uint16_t id, float value);
bool ulDatabase_getFloat(struct ulDatabase * self, uint16_t id, float *value);
bool ulDatabase_reset(struct ulDatabase * self, uint16_t id);
struct ulDatabase_ParamMetadata *ulDatabase_getMetadata(struct ulDatabase * self, uint16_t id);
bool ulDatabase_validateId(struct ulDatabase * self, uint16_t id);
