/******************************************************************************************
 * File:        logger.h
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

#ifndef _LOGGER_h
#define _LOGGER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

// Constants
#define LOGGER_NO_ERROR (0)
#define LOGGER_ERROR_FAILED_NB_MAX_SLOTS (-1)
#define LOGGER_ERROR_SLOT_ID_NOT_FOUND (-2)
#define LOGGER_ERROR_NO_FREE_SLOT (-3)
#define LOGGER_NO_FILLED_SLOT (-3)
#define LOGGER_ERROR_FAILED_ALLOCATE_MEMORY (-4)

#define LOGGER_NB_SLOTS (100)

typedef enum
{
    LOGGER_SLOT_STATUS_EMPTY,
    LOGGER_SLOT_STATUS_WAITING_TRANSMIT,
    LOGGER_SLOT_STATUS_QUEUED,
    LOGGER_SLOT_STATUS_TRANSMITTED,
} logger_slot_status_id_t;

int logger_init(void);
int logger_term(void);

int logger_insert_data(void *buffer, uint16_t size, uint8_t tag, uint32_t createdDate, uint16_t *id);
int logger_get_data(uint16_t id, void **buffer, uint16_t *size, uint8_t *tag, uint32_t *createdDate, uint32_t *acknowledgedDate, logger_slot_status_id_t *status);

int logger_clear_slot(uint16_t id);
int logger_clear_all_slots(void);
int logger_clear_all_slots_matching_tag(uint8_t tag);

int logger_get_number_free_msg_slots(uint16_t *count);

int logger_get_oldest_slot_id(uint16_t *id);
int logger_get_youngest_slot_id(uint16_t *id);
int logger_get_youngest_slot_id_older_than(uint16_t *id, uint32_t *createdDate, uint32_t thresh_epoch);

int logger_get_tag_from_slot_id(uint16_t id, uint8_t *tag);
int logger_set_status_of_slot_id(uint16_t id, logger_slot_status_id_t status);
int logger_set_acknowledgeddate_of_slot_id(uint16_t id, uint32_t acknowledgedDate);

void logger_get_total_size_bytes(size_t *total);

#endif