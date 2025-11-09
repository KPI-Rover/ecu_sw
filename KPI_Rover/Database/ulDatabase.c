#include "ulDatabase.h"

#include "FreeRTOS.h"

#define DB_LOCK() osMutexAcquire(ulDatabase_mutex, osWaitForever)
#define DB_FREE() osMutexRelease(ulDatabase_mutex)

static osMutexId_t ulDatabase_mutex;
StaticSemaphore_t *ulDatabase_mutex_cb;

static struct ulDatabase db;

bool ulDatabase_init(struct ulDatabase_ParamMetadata * metadataTable, uint16_t metadataCount)
{
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
	{
		void *db_mem = malloc(db_size);

		if (db_mem == NULL)
			return false;

		db.dataArray = (uint8_t *) db_mem;
	}

	// create global database lock (mutex) for the entire database
	{
		ulDatabase_mutex_cb = malloc(sizeof(StaticSemaphore_t));

		if (ulDatabase_mutex_cb == NULL)
			return false;

		osMutexAttr_t mutex_attrs = {NULL, 0, ulDatabase_mutex_cb, sizeof(StaticSemaphore_t)};

		if (NULL == (ulDatabase_mutex = osMutexNew(&mutex_attrs)))
			return false;
	}

	// save metadata for allocated db + metadata size
	db.dataArraySize = db_size;
	db.metadataCount = metadataCount;

	// save metadata table location
	db.metadataTable = metadataTable;

	// fill db memory with default values
	for (uint16_t i = 0; i < metadataCount; i++)
		ulDatabase_reset(i);

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
	*((uint8_t *) &(db.dataArray[p->offset])) = value;
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
	*value = *((uint8_t *) &(db.dataArray[p->offset]));
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
	*((int8_t *) &(db.dataArray[p->offset])) = value;
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
	*value = *((int8_t *) &(db.dataArray[p->offset]));
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
	*((uint16_t *) &(db.dataArray[p->offset])) = value;
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
	*value = *((uint16_t *) &(db.dataArray[p->offset]));
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
	*((int16_t *) &(db.dataArray[p->offset])) = value;
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
	*value = *((int16_t *) &(db.dataArray[p->offset]));
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
	*((uint32_t *) &(db.dataArray[p->offset])) = value;
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
	*value = *((uint32_t *) &(db.dataArray[p->offset]));
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
	*((int32_t *) &(db.dataArray[p->offset])) = value;
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
	*value = *((int32_t *) &(db.dataArray[p->offset]));
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
	*((float *) &(db.dataArray[p->offset])) = value;
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
	*value = *((float *) &(db.dataArray[p->offset]));
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
		*((uint8_t *) &(db.dataArray[p->offset])) = p->defaultValue;
		break;
	case INT8:
		*((int8_t *) &(db.dataArray[p->offset])) = p->defaultValue;
		break;
	case UINT16:
		*((uint16_t *) &(db.dataArray[p->offset])) = p->defaultValue;
		break;
	case INT16:
		*((int16_t *) &(db.dataArray[p->offset])) = p->defaultValue;
		break;
	case UINT32:
		*((uint32_t *) &(db.dataArray[p->offset])) = p->defaultValue;
		break;
	case INT32:
		*((int32_t *) &(db.dataArray[p->offset])) = p->defaultValue;
		break;
	case FLOAT:
		*((float *) &(db.dataArray[p->offset])) = p->defaultValue;
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
