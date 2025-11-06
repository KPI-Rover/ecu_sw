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

uint16_t ulDatabase_ParamMetadata_getSize(struct ulDatabase_ParamMetadata * self);

struct ulDatabase {
	uint8_t *dataArray;
	struct ulDatabase_ParamMetadata *metadataTable;
	uint16_t metadataCount;
	uint16_t dataArraySize;
};

/**
 * @brief Initialize the database with the given metadata table.
 * @param metadataTable Pointer to the parameter metadata table.
 * @param metadataCount Number of entries in the metadata table.
 * @return true if initialization succeeded, false on failure.
 */
bool ulDatabase_init(struct ulDatabase_ParamMetadata * metadataTable, uint16_t metadataCount);

/**
 * @brief Set the value of a parameter as uint8_t.
 * @param id Parameter ID.
 * @param value Value to set.
 * @return true if the value was set successfully, false otherwise.
 */
bool ulDatabase_setUint8(uint16_t id, uint8_t value);

/**
 * @brief Get the value of a parameter as uint8_t.
 * @param id Parameter ID.
 * @param value Pointer to store the retrieved value.
 * @return true if the value was retrieved successfully, false otherwise.
 */
bool ulDatabase_getUint8(uint16_t id, uint8_t *value);

/**
 * @brief Set the value of a parameter as int8_t.
 * @param id Parameter ID.
 * @param value Value to set.
 * @return true if the value was set successfully, false otherwise.
 */
bool ulDatabase_setInt8(uint16_t id, int8_t value);

/**
 * @brief Get the value of a parameter as int8_t.
 * @param id Parameter ID.
 * @param value Pointer to store the retrieved value.
 * @return true if the value was retrieved successfully, false otherwise.
 */
bool ulDatabase_getInt8(uint16_t id, int8_t *value);

/**
 * @brief Set the value of a parameter as uint16_t.
 * @param id Parameter ID.
 * @param value Value to set.
 * @return true if the value was set successfully, false otherwise.
 */
bool ulDatabase_setUint16(uint16_t id, uint16_t value);

/**
 * @brief Get the value of a parameter as uint16_t.
 * @param id Parameter ID.
 * @param value Pointer to store the retrieved value.
 * @return true if the value was retrieved successfully, false otherwise.
 */
bool ulDatabase_getUint16(uint16_t id, uint16_t *value);

/**
 * @brief Set the value of a parameter as int16_t.
 * @param id Parameter ID.
 * @param value Value to set.
 * @return true if the value was set successfully, false otherwise.
 */
bool ulDatabase_setInt16(uint16_t id, int16_t value);

/**
 * @brief Get the value of a parameter as int16_t.
 * @param id Parameter ID.
 * @param value Pointer to store the retrieved value.
 * @return true if the value was retrieved successfully, false otherwise.
 */
bool ulDatabase_getInt16(uint16_t id, int16_t *value);

/**
 * @brief Set the value of a parameter as uint32_t.
 * @param id Parameter ID.
 * @param value Value to set.
 * @return true if the value was set successfully, false otherwise.
 */
bool ulDatabase_setUint32(uint16_t id, uint32_t value);

/**
 * @brief Get the value of a parameter as uint32_t.
 * @param id Parameter ID.
 * @param value Pointer to store the retrieved value.
 * @return true if the value was retrieved successfully, false otherwise.
 */
bool ulDatabase_getUint32(uint16_t id, uint32_t *value);

/**
 * @brief Set the value of a parameter as int32_t.
 * @param id Parameter ID.
 * @param value Value to set.
 * @return true if the value was set successfully, false otherwise.
 */
bool ulDatabase_setInt32(uint16_t id, int32_t value);

/**
 * @brief Get the value of a parameter as int32_t.
 * @param id Parameter ID.
 * @param value Pointer to store the retrieved value.
 * @return true if the value was retrieved successfully, false otherwise.
 */
bool ulDatabase_getInt32(uint16_t id, int32_t *value);

/**
 * @brief Set the value of a parameter as float.
 * @param id Parameter ID.
 * @param value Value to set.
 * @return true if the value was set successfully, false otherwise.
 */
bool ulDatabase_setFloat(uint16_t id, float value);

/**
 * @brief Get the value of a parameter as float.
 * @param id Parameter ID.
 * @param value Pointer to store the retrieved value.
 * @return true if the value was retrieved successfully, false otherwise.
 */
bool ulDatabase_getFloat(uint16_t id, float *value);

/**
 * @brief Reset the parameter to its default value.
 * @param id Parameter ID.
 * @return true if the parameter was reset successfully, false otherwise.
 */
bool ulDatabase_reset(uint16_t id);

/**
 * @brief Reset all parameters to their default values.
 * @return true if all parameters were reset successfully, false otherwise.
 */
bool ulDatabase_resetAll(void);
