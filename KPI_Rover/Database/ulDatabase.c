#include "ulDatabase.h"

#include "FreeRTOS.h"

#define DB_LOCK(id) osMutexAcquire(mutexes[id], osWaitForever)
#define DB_FREE(id) osMutexRelease(mutexes[id])

osMutexId_t * mutexes;
StaticSemaphore_t *mutexes_cb;

bool ulDatabase_init(struct ulDatabase * self, struct ulDatabase_ParamMetadata * metadataTable, uint16_t metadataCount)
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

		self->dataArray = (uint8_t *) db_mem;
	}

	// create mutexes for every parameter
	{
		mutexes = malloc(sizeof(osMutexId_t) * metadataCount);

		if (mutexes == NULL)
			return false;

		mutexes_cb = malloc(sizeof(StaticSemaphore_t) * metadataCount);

		if (mutexes_cb == NULL)
			return false;

		for (uint16_t i = 0; i < metadataCount; i++) {
			osMutexAttr_t mutex_attrs = {NULL, 0, &(mutexes_cb[i]), sizeof(StaticSemaphore_t)};

			if (NULL == (mutexes[i] = osMutexNew(&mutex_attrs)))
				return false;
		}
	}

	// fill db memory with default values
	for (uint16_t i = 0; i < metadataCount; i++)
		ulDatabase_reset(self, i);

	// save metadata for allocated db + metadata size
	self->dataArraySize = db_size;
	self->metadataCount = metadataCount;

	// save metadata table location
	self->metadataTable = metadataTable;

	return true;
}

bool ulDatabase_setUint8(struct ulDatabase * self, uint16_t id, uint8_t value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(self, id);

	if (p == NULL)
		return false;

	if (p->type != UINT8)
		return false;

	DB_LOCK(id);
	*((uint8_t *) &(self->dataArray[p->offset])) = value;
	DB_FREE(id);

	return true;
}

bool ulDatabase_getUint8(struct ulDatabase * self, uint16_t id, uint8_t *value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(self, id);

	if (p == NULL)
		return false;

	if (p->type != UINT8)
		return false;

	DB_LOCK(id);
	*value = *((uint8_t *) &(self->dataArray[p->offset]));
	DB_FREE(id);

	return true;
}

bool ulDatabase_setInt8(struct ulDatabase * self, uint16_t id, int8_t value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(self, id);

	if (p == NULL)
		return false;

	if (p->type != INT8)
		return false;

	DB_LOCK(id);
	*((int8_t *) &(self->dataArray[p->offset])) = value;
	DB_FREE(id);

	return true;
}

bool ulDatabase_getInt8(struct ulDatabase * self, uint16_t id, int8_t *value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(self, id);

	if (p == NULL)
		return false;

	if (p->type != INT8)
		return false;

	DB_LOCK(id);
	*value = *((int8_t *) &(self->dataArray[p->offset]));
	DB_FREE(id);

	return true;
}

bool ulDatabase_setUint16(struct ulDatabase * self, uint16_t id, uint16_t value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(self, id);

	if (p == NULL)
		return false;

	if (p->type != UINT16)
		return false;

	DB_LOCK(id);
	*((uint16_t *) &(self->dataArray[p->offset])) = value;
	DB_FREE(id);

	return true;
}

bool ulDatabase_getUint16(struct ulDatabase * self, uint16_t id, uint16_t *value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(self, id);

	if (p == NULL)
		return false;

	if (p->type != UINT16)
		return false;

	DB_LOCK(id);
	*value = *((uint16_t *) &(self->dataArray[p->offset]));
	DB_FREE(id);

	return true;
}

bool ulDatabase_setInt16(struct ulDatabase * self, uint16_t id, int16_t value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(self, id);

	if (p == NULL)
		return false;

	if (p->type != UINT16)
		return false;

	DB_LOCK(id);
	*((int16_t *) &(self->dataArray[p->offset])) = value;
	DB_FREE(id);

	return true;
}

bool ulDatabase_getInt16(struct ulDatabase * self, uint16_t id, int16_t *value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(self, id);

	if (p == NULL)
		return false;

	if (p->type != UINT16)
		return false;

	DB_LOCK(id);
	*value = *((int16_t *) &(self->dataArray[p->offset]));
	DB_FREE(id);

	return true;
}

bool ulDatabase_setUint32(struct ulDatabase * self, uint16_t id, uint32_t value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(self, id);

	if (p == NULL)
		return false;

	if (p->type != UINT16)
		return false;

	DB_LOCK(id);
	*((uint32_t *) &(self->dataArray[p->offset])) = value;
	DB_FREE(id);

	return true;
}

bool ulDatabase_getUint32(struct ulDatabase * self, uint16_t id, uint32_t *value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(self, id);

	if (p == NULL)
		return false;

	if (p->type != UINT16)
		return false;

	DB_LOCK(id);
	*value = *((uint32_t *) &(self->dataArray[p->offset]));
	DB_FREE(id);

	return true;
}

bool ulDatabase_setInt32(struct ulDatabase * self, uint16_t id, int32_t value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(self, id);

	if (p == NULL)
		return false;

	if (p->type != UINT16)
		return false;

	DB_LOCK(id);
	*((int32_t *) &(self->dataArray[p->offset])) = value;
	DB_FREE(id);

	return true;
}

bool ulDatabase_getInt32(struct ulDatabase * self, uint16_t id, int32_t *value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(self, id);

	if (p == NULL)
		return false;

	if (p->type != UINT16)
		return false;

	DB_LOCK(id);
	*value = *((int32_t *) &(self->dataArray[p->offset]));
	DB_FREE(id);

	return true;
}

bool ulDatabase_setFloat(struct ulDatabase * self, uint16_t id, float value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(self, id);

	if (p == NULL)
		return false;

	if (p->type != UINT16)
		return false;

	DB_LOCK(id);
	*((float *) &(self->dataArray[p->offset])) = value;
	DB_FREE(id);

	return true;
}

bool ulDatabase_getFloat(struct ulDatabase * self, uint16_t id, float *value)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(self, id);

	if (p == NULL)
		return false;

	if (p->type != UINT16)
		return false;

	DB_LOCK(id);
	*value = *((float *) &(self->dataArray[p->offset]));
	DB_FREE(id);

	return true;
}

bool ulDatabase_reset(struct ulDatabase * self, uint16_t id)
{
	struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(self, id);

	if (p == NULL)
		return false;

	DB_LOCK(id);

	switch (p->type) {
	case UINT8:
		*((uint8_t *) &(self->dataArray[p->offset])) = p->defaultValue;
		break;
	case INT8:
		*((int8_t *) &(self->dataArray[p->offset])) = p->defaultValue;
		break;
	case UINT16:
		*((uint16_t *) &(self->dataArray[p->offset])) = p->defaultValue;
		break;
	case INT16:
		*((int16_t *) &(self->dataArray[p->offset])) = p->defaultValue;
		break;
	case UINT32:
		*((uint32_t *) &(self->dataArray[p->offset])) = p->defaultValue;
		break;
	case INT32:
		*((int32_t *) &(self->dataArray[p->offset])) = p->defaultValue;
		break;
	case FLOAT:
		*((float *) &(self->dataArray[p->offset])) = p->defaultValue;
		break;
	}

	DB_FREE(id);

	return true;
}

struct ulDatabase_ParamMetadata *ulDatabase_getMetadata(struct ulDatabase * self, uint16_t id)
{
	if (!ulDatabase_validateId(self, id))
		return NULL;

	return &(self->metadataTable[id]);
}

bool ulDatabase_validateId(struct ulDatabase * self, uint16_t id)
{
	return self->metadataCount > id;
}
