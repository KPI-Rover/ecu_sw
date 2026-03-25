#include <string.h>

#include "ulDatabase.h"
#include "ulStorage.h"

#include "ulog.h"

extern char _sccmram[]; // value exported by linker script

#define PAGE_START _sccmram	// CCM RAM (to protect actual flash memory)
#define PAGE_SIZE 0x4000	// model 16KiB FLASH memory
#define MARKER_SAVE_BEGIN 0xAA

static uint32_t persistent_db_field_size;


// flash emulation section start
static bool flash_unlocked;

static void HAL_FLASH_Unlock(void)
{
	flash_unlocked = 1;
}

static void HAL_FLASH_Lock(void)
{
	flash_unlocked = 0;
}

enum Size {
	FLASH_TYPEPROGRAM_BYTE
};

static void HAL_FLASH_Erase(void)
{
	if (!flash_unlocked) {
		ULOG_WARNING("Erase command on locked flash");
		return;
	}

	memset((void *) PAGE_START, 0xFF, PAGE_SIZE);
}

static void HAL_FLASH_Program(enum Size s, const void * const addr, const uint64_t value)
{
	(void) s;

	if (!flash_unlocked) {
		ULOG_WARNING("Write command on locked flash: %p <- 0x%02hhx", addr, (uint8_t) value);
		return;
	}

	if ((addr >= (void *) (PAGE_START + PAGE_SIZE)) || (addr < (void *) PAGE_START)) {
		ULOG_CRITICAL("OUT OF BOUNDS FLASH WRITE GENERATED");
		ULOG_CRITICAL("TRIED TO WRITE VALUE 0x%02hhx TO ADDRESS %p", (uint8_t) value, addr);
		return;
	}

	*(uint8_t *) addr = (uint8_t) value;
}
// flash emulation section finish

static uint32_t crc32(const uint8_t * const data, const uint32_t data_size, const uint32_t remainder)
{
    register uint32_t reg_a = 0,
                      reg_b = 0;

    static const uint32_t divisor = 0x04C11DB7;

    uint32_t read_offset = 0;
    uint32_t xor_ready = 0;

    while ((data_size - read_offset) >= 4) {
        reg_b = *(uint32_t *) &(data[read_offset]);
        read_offset += 4;

        for (unsigned int i = 0; i < 32; i++) {
            // check if XOR can be done yet
            xor_ready = reg_a & 0x80000000;

            // shift in
            reg_a <<= 1;

            if (reg_b & 0x80000000)
                reg_a |= 0x1;

            reg_b <<= 1;

            // XOR if ready
            if (xor_ready)
                reg_a ^= divisor;
        }
    }

    if (data_size - read_offset != 0) {
        // fill reg_b
        for (unsigned int i = 0; i < (data_size - read_offset); i++) {
            reg_b |= data[read_offset + i];
            reg_b <<= 8;
        }

        reg_b <<= 8 * (3 - (data_size - read_offset));

        // finish calculating main part
        for (unsigned int i = 0; i < 8 * (data_size - read_offset); i++) {
            // check if XOR can be done yet
            xor_ready = reg_a & 0x80000000;

            // shift in
            reg_a <<= 1;

            if (reg_b & 0x80000000)
                reg_a |= 0x1;

            reg_b <<= 1;

            // XOR if ready
            if (xor_ready)
                reg_a ^= divisor;
        }
    }

    // now finish with 32 bits of the remainder
    reg_b = remainder;

    // finish calculating main part
    for (unsigned int i = 0; i < 32; i++) {
        // check if XOR can be done yet
        xor_ready = reg_a & 0x80000000;

        // shift in
        reg_a <<= 1;

        if (reg_b & 0x80000000)
            reg_a |= 0x1;

        reg_b <<= 1;

        // XOR if ready
        if (xor_ready)
            reg_a ^= divisor;
    }

    return reg_a;
}

static uint32_t crc32_calculate(const uint8_t * const data, const uint32_t data_size)
{
    return crc32(data, data_size, 0);
}

static uint32_t crc32_verify(const uint8_t * const data, const uint32_t data_size, const uint32_t remainder)
{
    return crc32(data, data_size, remainder);
}

static uint32_t ulStorage_get_save_size(void)
{
	uint32_t persistent_fields_len = 0;

	for (uint16_t i = 0; i < PARAM_COUNT; i++) {
		struct ulDatabase_ParamMetadata *p = ulDatabase_getMetadata(i);

		if (p == NULL) {
			ULOG_ERROR("p == NULL, but parameter should exist (i = %hu, PARAM_COUNT = %hu); this is not possible, disabling ulStorage to prevent further damage",
					i, (uint16_t) PARAM_COUNT);
			return 0;
		}

		if (!p->persistent)
			break;

		switch (p->type) {
		case INT8:
		case UINT8:
			persistent_fields_len += 1;
			break;
		case INT16:
		case UINT16:
			persistent_fields_len += 2;
			break;
		case INT32:
		case UINT32:
		case FLOAT:
			persistent_fields_len += 4;
			break;
		}
	}

	return persistent_fields_len;
}

static bool ulStorage_verify_sector_header(void)
{
	uint32_t sector_header = *(uint32_t *) PAGE_START;

	// verify the sector is erased
	if (sector_header & 0x80) {
		ULOG_WARNING("Sector is not formatted: ERASED flag is 1");
		return false;
	}

	// verify no invalid flags are present
	if ((sector_header & 0x2F) != 0x2F) {
		ULOG_WARNING("Sector is not formatted: garbage flags found");
		return false;
	}

	// verify save size matches current persistent field size
	if ((sector_header >> 20) != persistent_db_field_size) {
		ULOG_WARNING("Save size does not match with current size");
		return false;
	}

	// verify hash is not there yet
	if ((sector_header & 0x000FFF00) != 0x000FFF00) {
		ULOG_WARNING("Garbage found in HASH header field");
		return false;
	}

	return true;
}

static uint8_t *ulStorage_find_first_free_block(void)
{
	uint8_t *seek_head;

	for (
		seek_head = (uint8_t *) PAGE_START + 4;
		(seek_head < (uint8_t *) (PAGE_START + PAGE_SIZE)) && (*seek_head != 0xFF);
		seek_head += 1 + persistent_db_field_size + 4);

	return seek_head;
}

static bool ulStorage_find_last_save(uint8_t **buffer)
{
	if (!ulStorage_verify_sector_header())
		return false;

	uint8_t *seek_head = ulStorage_find_first_free_block();

	// finding last valid save data
	uint32_t save_checksum = 0xFFFFFFFF;

	while ((seek_head > (uint8_t *) PAGE_START + 4) && save_checksum) {
		seek_head -= 1 + persistent_db_field_size + 4;
		ULOG_DEBUG("Verifying save hash for %p", seek_head);

		save_checksum = crc32_verify(seek_head + 1, persistent_db_field_size, *(uint32_t *)(seek_head + 1 + persistent_db_field_size));
		ULOG_DEBUG("Result: 0x%08X", save_checksum);
	}

	// if none found, stop
	if (save_checksum) {
		ULOG_WARNING("No valid saves found");
		return false;
	}

	// otherwise set the correct pointer
	*buffer = seek_head + 1;

	return true;
}

static bool ulStorage_find_new_save_block(uint8_t ** block)
{
	if (!ulStorage_verify_sector_header())
		return false;

	uint8_t *new_block = ulStorage_find_first_free_block();

	if ((new_block + 1 + persistent_db_field_size + 4) >= (uint8_t *) (PAGE_START + PAGE_SIZE))
		return false;

	*block = new_block;

	return true;
}

static void ulStorage_format_sector(void)
{
	uint32_t new_header = 0xFFFFFFFF;

	// mark ERASED bit
	new_header &= 0xFFFFFF7F;

	// mark save size
	new_header &= (persistent_db_field_size << 20) | (0x000FFFFF);

	// --- ENTER UNSAFE ZONE ---

	HAL_FLASH_Unlock();

	HAL_FLASH_Erase();

	// write header to flash
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, (void *) (PAGE_START + 0), (new_header >>  0) & 0xFF);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, (void *) (PAGE_START + 1), (new_header >>  8) & 0xFF);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, (void *) (PAGE_START + 2), (new_header >> 16) & 0xFF);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, (void *) (PAGE_START + 3), (new_header >> 24) & 0xFF);

	HAL_FLASH_Lock();

	// --- LEAVE UNSAFE ZONE ---
}

static void ulStorage_write_save(uint8_t * const write_at, const uint8_t * const save_data)
{
	uint32_t checksum = crc32_calculate(save_data, persistent_db_field_size);

	// --- ENTER UNSAFE ZONE ---

	HAL_FLASH_Unlock();

	HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, write_at, MARKER_SAVE_BEGIN);

	for (uint32_t i = 0; i < persistent_db_field_size; i++)
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, write_at + 1 + i, save_data[i]);

	for (uint32_t i = 0; i < 4; i++)
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, write_at + 1 + persistent_db_field_size + i, (checksum >> (8 * i)) & 0xFF);

	HAL_FLASH_Lock();

	// --- LEAVE UNSAFE ZONE ---
}

bool ulStorage_init(void)
{
	persistent_db_field_size = ulStorage_get_save_size();

	bool r = ulStorage_load();
	if (r) {
		ULOG_INFO("Loaded save from storage");
	} else {
		ULOG_INFO("Loading save from storage has failed");
	}

	return true;
}

bool ulStorage_load(void)
{
	if (!persistent_db_field_size)
		return true;

	uint8_t *buffer;

	if (!ulStorage_find_last_save(&buffer))
		return false;

	uint8_t *db_data = ulDatabase_freeze();
	memcpy(db_data, buffer, persistent_db_field_size);
	ulDatabase_unfreeze();

	return true;
}

bool ulStorage_save(void)
{
	if (!persistent_db_field_size)
		return true;

	// get a copy of persistent fields
	uint8_t new_save_data[persistent_db_field_size];

	uint8_t *db_data = ulDatabase_freeze();
	memcpy(new_save_data, db_data, persistent_db_field_size);
	ulDatabase_unfreeze();

	// try to load last save
	uint8_t *last_save_data;
	if (ulStorage_find_last_save(&last_save_data))
		if (!memcmp(new_save_data, last_save_data, persistent_db_field_size)) {
			ULOG_INFO("Old save is valid and identical to a new one; not writing another block");
			return true;
		}

	// write a new save
	uint8_t *new_save_address;

	if (!ulStorage_find_new_save_block(&new_save_address)) {
		// the storage is not formatted or is full - either way needs formatting
		ULOG_INFO("Sector is full or not formatted; formatting the sector and writing new save");
		ulStorage_format_sector();
		ulStorage_write_save((uint8_t *) (PAGE_START + 4), new_save_data);
	} else {
		ULOG_INFO("Writing new save to %p", new_save_address);
		ulStorage_write_save(new_save_address, new_save_data);
	}

	return true;
}

bool ulStorage_erase(void)
{
	if (!persistent_db_field_size)
		return true;

	ulStorage_format_sector();

	return true;
}

bool ulStorage_factoryReset(void)
{
	for (uint16_t i = 0; ulDatabase_reset(i); i++);

	if (!persistent_db_field_size)
		return true;

	ulStorage_format_sector();

	return true;
}
