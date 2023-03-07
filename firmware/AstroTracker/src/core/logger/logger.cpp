/******************************************************************************************
 * File:        logger.cpp
 * Author:      valcesch
 * Compagny:    NA
 * Website:     https://github.com/valcesch/AstroTracker
 * E-mail:      NA
 *
 * AstroTracker
 * Copyright (C) 2023 valcesch
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************************/

#include "logger.h"
#include "../debug/debug.h"
#include "../../syshal/syshal_rtc.h"

typedef struct __attribute__((__packed__))
{
	uint16_t id = 0;
	uint8_t tag;
	uint32_t createdDate = 0;
	uint32_t acknowledgedDate = 0;
	size_t buffer_size;
	uint8_t *buffer;
	logger_slot_status_id_t status;
} LOGGER_Struct;

LOGGER_Struct *logger_struct;

int logger_init(void)
{
	DEBUG_PR_INFO("Using internal storage. %s", __FUNCTION__);

	// Allocate base logger table
	logger_struct = (LOGGER_Struct *)calloc(LOGGER_NB_SLOTS, sizeof(LOGGER_Struct));

	if (logger_struct == NULL)
	{
		DEBUG_PR_ERROR("Not enough memory for allocation. %s", __FUNCTION__);
		return LOGGER_ERROR_FAILED_ALLOCATE_MEMORY;
	}

	logger_clear_all_slots();

	return LOGGER_NO_ERROR;
}

int logger_term(void)
{
	for (uint16_t i = 0; i < LOGGER_NB_SLOTS; i++)
	{
		free(logger_struct[i].buffer);
	}

	free(logger_struct);

	return LOGGER_NO_ERROR;
}

int logger_insert_data(void *buffer, uint16_t size, uint8_t tag, uint32_t createdDate, uint16_t *id)
{
	for (uint16_t i = 0; i < LOGGER_NB_SLOTS; i++)
	{
		if (logger_struct[i].id == 0)
		{
			logger_struct[i].id = i + 1; // 0 reserved for empty solts
			logger_struct[i].tag = tag;
			logger_struct[i].createdDate = createdDate;
			logger_struct[i].buffer_size = size;
			logger_struct[i].buffer = (uint8_t *)calloc(size, sizeof(uint8_t));
			logger_struct[i].status = LOGGER_SLOT_STATUS_WAITING_TRANSMIT;

			if (logger_struct[i].buffer == NULL)
			{
				DEBUG_PR_ERROR("Not enough memory for allocation. %s", __FUNCTION__);
				return LOGGER_ERROR_FAILED_ALLOCATE_MEMORY;
			}

			memcpy(logger_struct[i].buffer, buffer, size);
			*id = logger_struct[i].id;

			DEBUG_PR_TRACE("Create slot: %d. %s", logger_struct[i].id, __FUNCTION__);

			return LOGGER_NO_ERROR;
		}
	}

	return LOGGER_ERROR_NO_FREE_SLOT;
}

int logger_get_data(uint16_t id, void **buffer, uint16_t *size, uint8_t *tag, uint32_t *createdDate, uint32_t *acknowledgedDate, logger_slot_status_id_t *status)
{
	for (uint16_t i = 0; i < LOGGER_NB_SLOTS; i++)
	{
		if (logger_struct[i].id == id)
		{
			*createdDate = logger_struct[i].createdDate;
			*acknowledgedDate = logger_struct[i].acknowledgedDate;
			*tag = logger_struct[i].tag;
			*size = logger_struct[i].buffer_size;
			*buffer = logger_struct[i].buffer;
			*status = logger_struct[i].status;

			return LOGGER_NO_ERROR;
		}
	}

	return LOGGER_ERROR_SLOT_ID_NOT_FOUND;
}

int logger_clear_slot(uint16_t id)
{
	for (uint16_t i = 0; i < LOGGER_NB_SLOTS; i++) // Check if slot id != 0
	{
		if (logger_struct[i].id == id)
		{
			logger_struct[i].id = 0;
			logger_struct[i].tag = 0;
			logger_struct[i].createdDate = 0;
			logger_struct[i].buffer_size = 0;
			logger_struct[i].status = LOGGER_SLOT_STATUS_EMPTY;
			free(logger_struct[i].buffer);

			return LOGGER_NO_ERROR;
		}
	}

	return LOGGER_ERROR_SLOT_ID_NOT_FOUND;
}

int logger_clear_all_slots_matching_tag(uint8_t tag)
{
	for (uint16_t i = 0; i < LOGGER_NB_SLOTS; i++) // Check if slot id != 0
	{
		if (logger_struct[i].tag == tag)
		{
			logger_clear_slot(logger_struct[i].id);
		}
	}

	return LOGGER_NO_ERROR;
}

int logger_clear_all_slots(void)
{
	for (uint16_t i = 0; i < LOGGER_NB_SLOTS; i++)
	{
		logger_clear_slot(i);
	}

	return LOGGER_NO_ERROR;
}

int logger_get_number_free_msg_slots(uint16_t *count)
{
	*count = 0;

	for (uint16_t i = 0; i < LOGGER_NB_SLOTS; i++)
	{
		if (logger_struct[i].id == 0)
		{
			*count++;
		}
	}

	return LOGGER_NO_ERROR;
}

int logger_get_oldest_slot_id(uint16_t *id)
{
	*id = 0;

	uint32_t oldest_slot_epoch = 0xFFFFFFFF;

	for (uint16_t i = 0; i < LOGGER_NB_SLOTS; i++)
	{
		if ((logger_struct[i].id != 0) &&
			(logger_struct[i].createdDate <= oldest_slot_epoch))
		{
			oldest_slot_epoch = logger_struct[i].createdDate;
			*id = logger_struct[i].id;
		}
	}

	if (*id == 0)
	{
		return LOGGER_NO_FILLED_SLOT;
	}

	return LOGGER_NO_ERROR;
}

int logger_get_youngest_slot_id(uint16_t *id)
{
	*id = 0;

	uint32_t youngest_slot_epoch = 0x00000000;

	for (uint16_t i = 0; i < LOGGER_NB_SLOTS; i++)
	{
		if ((logger_struct[i].id != 0) &&
			(logger_struct[i].createdDate >= youngest_slot_epoch))
		{
			youngest_slot_epoch = logger_struct[i].createdDate;
			*id = logger_struct[i].id;
		}
	}

	if (*id == 0)
	{
		return LOGGER_NO_FILLED_SLOT;
	}

	return LOGGER_NO_ERROR;
}

int logger_get_youngest_slot_id_older_than(uint16_t *id, uint32_t *createdDate, uint32_t thresh_epoch)
{
	*id = 0;

	uint32_t youngest_slot_epoch = 0x00000000;

	for (uint16_t i = 0; i < LOGGER_NB_SLOTS; i++)
	{
		if ((logger_struct[i].id != 0) &&
			(logger_struct[i].createdDate >= youngest_slot_epoch) &&
			(logger_struct[i].createdDate < thresh_epoch))
		{
			youngest_slot_epoch = logger_struct[i].createdDate;
			*id = logger_struct[i].id;
			*createdDate = logger_struct[i].createdDate;
		}
	}

	if (*id == 0)
	{
		return LOGGER_NO_FILLED_SLOT;
	}

	return LOGGER_NO_ERROR;
}

int logger_get_tag_from_slot_id(uint16_t id, uint8_t *tag)
{
	for (uint16_t i = 0; i < LOGGER_NB_SLOTS; i++)
	{
		if (logger_struct[i].id == id)
		{
			*tag = logger_struct[i].tag;
			return LOGGER_NO_ERROR;
		}
	}
	return LOGGER_ERROR_SLOT_ID_NOT_FOUND;
}

int logger_set_status_of_slot_id(uint16_t id, logger_slot_status_id_t status)
{
	for (uint16_t i = 0; i < LOGGER_NB_SLOTS; i++)
	{
		if (logger_struct[i].id == id)
		{
			logger_struct[i].status = status;
			return LOGGER_NO_ERROR;
		}
	}
	return LOGGER_ERROR_SLOT_ID_NOT_FOUND;
}

int logger_set_acknowledgeddate_of_slot_id(uint16_t id, uint32_t acknowledgedDate)
{
	for (uint16_t i = 0; i < LOGGER_NB_SLOTS; i++)
	{
		if (logger_struct[i].id == id)
		{
			logger_struct[i].acknowledgedDate = acknowledgedDate;
			return LOGGER_NO_ERROR;
		}
	}
	return LOGGER_ERROR_SLOT_ID_NOT_FOUND;
}

void logger_get_total_size_bytes(size_t *total)
{
	for (uint16_t i = 0; i < LOGGER_NB_SLOTS; i++)
	{
		if (logger_struct[i].id != 0)
		{
			*total += sizeof(LOGGER_Struct) + logger_struct[i].buffer_size;
		}
	}
}
