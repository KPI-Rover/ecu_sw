#include <string.h>

#include "ulDatabase.h"

#include "FreeRTOS.h"

#include "ulog.h"

#define DB_LOCK() osMutexAcquire(ulDatabase_mutex, osWaitForever)
#define DB_FREE() osMutexRelease(ulDatabase_mutex)

static osMutexId_t ulDatabase_mutex;
StaticSemaphore_t *ulDatabase_mutex_cb;

static struct ulDatabase db;

bool ulDatabase_init(struct ulDatabase_ParamMetadata * metadataTable, uint16_t metadataCount)
{
	ULOG_DEBUG("Database init start");

	// calculate DB size and assign correct offsets
	uint16_t db_size = 0;

	for (size_t i = 0; i < metadataCount; i++) {
		metadataTable[i].offset = db_size;

		switch (metadataTable[i].type) {
		case UINT8:
		case INT8:
			db_size += 1;
			break;
		case UINT16:
		case INT16:
			db_size += 2;
			break;
		case UINT32:
		case INT32:
		case FLOAT:
			db_size += 4;
			break;
		}
	}

	// allocate memory for DB
	if (db_size) {
		ULOG_INFO("Allocating %hu for database storage", db_size);

		void *db_mem = malloc(db_size);

		if (db_mem == NULL) {
			ULOG_ERROR("Failed to allocate memory for database storage: allocator returned NULL");
			return false;
		}

		db.dataArray = (uint8_t *) db_mem;
	} else {
		ULOG_INFO("Not allocating memory for an empty database");
		db.dataArray = NULL;
	}

	// create global database lock (mutex) for the entire database
	{
		ulDatabase_mutex_cb = malloc(sizeof(StaticSemaphore_t));

		if (ulDatabase_mutex_cb == NULL) {
			ULOG_ERROR("Failed to allocate memory for database mutex: allocator returned NULL");
			return false;
		}

		osMutexAttr_t mutex_attrs = {NULL, 0, ulDatabase_mutex_cb, sizeof(StaticSemaphore_t)};

		if (NULL == (ulDatabase_mutex = osMutexNew(&mutex_attrs))) {
			ULOG_ERROR("Failed to initialize mutex: OS error");
			return false;
		}
	}

	// save metadata for allocated db + metadata size
	db.dataArraySize = db_size;
	db.metadataCount = metadataCount;

	// save metadata table location
	db.metadataTable = metadataTable;

	// fill db memory with default values
	for (uint16_t i = 0; i < metadataCount; i++)
		ulDatabase_reset(i);

	ULOG_DEBUG("Database init finish [success]");

	return true;
}

bool ulDatabase_setUint8(uint16_t id, uint8_t value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(id);

	if (p == NULL)
		return false;

	if (p->type != UINT8)
		return false;

	DB_LOCK();
	memcpy(&(db.dataArray[p->offset]), &value, 1);
	DB_FREE();

	return true;
}

bool ulDatabase_getUint8(uint16_t id, uint8_t *value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(id);

	if (p == NULL)
		return false;

	if (p->type != UINT8)
		return false;

	DB_LOCK();
	memcpy(value, &(db.dataArray[p->offset]), 1);
	DB_FREE();

	return true;
}

bool ulDatabase_setInt8(uint16_t id, int8_t value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(id);

	if (p == NULL)
		return false;

	if (p->type != INT8)
		return false;

	DB_LOCK();
	memcpy(&(db.dataArray[p->offset]), &value, 1);
	DB_FREE();

	return true;
}

bool ulDatabase_getInt8(uint16_t id, int8_t *value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(id);

	if (p == NULL)
		return false;

	if (p->type != INT8)
		return false;

	DB_LOCK();
	memcpy(value, &(db.dataArray[p->offset]), 1);
	DB_FREE();

	return true;
}

bool ulDatabase_setUint16(uint16_t id, uint16_t value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(id);

	if (p == NULL)
		return false;

	if (p->type != UINT16)
		return false;

	DB_LOCK();
	memcpy(&(db.dataArray[p->offset]), &value, 2);
	DB_FREE();

	return true;
}

bool ulDatabase_getUint16(uint16_t id, uint16_t *value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(id);

	if (p == NULL)
		return false;

	if (p->type != UINT16)
		return false;

	DB_LOCK();
	memcpy(value, &(db.dataArray[p->offset]), 2);
	DB_FREE();

	return true;
}

bool ulDatabase_setInt16(uint16_t id, int16_t value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(id);

	if (p == NULL)
		return false;

	if (p->type != INT16)
		return false;

	DB_LOCK();
	memcpy(&(db.dataArray[p->offset]), &value, 2);
	DB_FREE();

	return true;
}

bool ulDatabase_getInt16(uint16_t id, int16_t *value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(id);

	if (p == NULL)
		return false;

	if (p->type != INT16)
		return false;

	DB_LOCK();
	memcpy(value, &(db.dataArray[p->offset]), 2);
	DB_FREE();

	return true;
}

bool ulDatabase_setUint32(uint16_t id, uint32_t value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(id);

	if (p == NULL)
		return false;

	if (p->type != UINT32)
		return false;

	DB_LOCK();
	memcpy(&(db.dataArray[p->offset]), &value, 4);
	DB_FREE();

	return true;
}

bool ulDatabase_getUint32(uint16_t id, uint32_t *value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(id);

	if (p == NULL)
		return false;

	if (p->type != UINT32)
		return false;

	DB_LOCK();
	memcpy(value, &(db.dataArray[p->offset]), 4);
	DB_FREE();

	return true;
}

bool ulDatabase_setInt32(uint16_t id, int32_t value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(id);

	if (p == NULL)
		return false;

	if (p->type != INT32)
		return false;

	DB_LOCK();
	memcpy(&(db.dataArray[p->offset]), &value, 4);
	DB_FREE();

	return true;
}

bool ulDatabase_getInt32(uint16_t id, int32_t *value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(id);

	if (p == NULL)
		return false;

	if (p->type != INT32)
		return false;

	DB_LOCK();
	memcpy(value, &(db.dataArray[p->offset]), 4);
	DB_FREE();

	return true;
}

bool ulDatabase_setFloat(uint16_t id, float value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(id);

	if (p == NULL)
		return false;

	if (p->type != FLOAT)
		return false;

	DB_LOCK();
	memcpy(&(db.dataArray[p->offset]), &value, 4);
	DB_FREE();

	return true;
}

bool ulDatabase_getFloat(uint16_t id, float *value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(id);

	if (p == NULL)
		return false;

	if (p->type != FLOAT)
		return false;

	DB_LOCK();
	memcpy(value, &(db.dataArray[p->offset]), 4);
	DB_FREE();

	return true;
}

bool ulDatabase_reset(uint16_t id)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(id);

	if (p == NULL)
		return false;

	DB_LOCK();

	switch (p->type) {
	case UINT8:
	case INT8:
		memcpy(&(db.dataArray[p->offset]), &(p->defaultValue), 1);
		break;
	case UINT16:
	case INT16:
		memcpy(&(db.dataArray[p->offset]), &(p->defaultValue), 2);
		break;
	case UINT32:
	case INT32:
	case FLOAT:
		memcpy(&(db.dataArray[p->offset]), &(p->defaultValue), 4);
		break;
	}

	DB_FREE();

	return true;
}

struct ulDatabase_ParamMetadata *ulDatabase_getMetadata(uint16_t id)
{
	if (!ulDatabase_validateId(id))
		return NULL;

	return &(db.metadataTable[id]);
}

bool ulDatabase_validateId(uint16_t id)
{
	return db.metadataCount > id;
}
