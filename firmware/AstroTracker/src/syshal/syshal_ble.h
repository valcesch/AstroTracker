/******************************************************************************************
 * File:        syshal_ble.h
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

#ifndef _SYSHAL_BLE_H_
#define _SYSHAL_BLE_H_

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "../core/config/sys_config.h"

// Constants
#define SYSHAL_BLE_MAX_BUFFER_SIZE (512)
#define SYSHAL_BLE_UUID_SIZE (16)
#define SYSHAL_BLE_ADVERTISING_SIZE (31)

#define SYSHAL_BLE_NO_ERROR (0)
#define SYSHAL_BLE_ERROR_CRC (-1)
#define SYSHAL_BLE_ERROR_TIMEOUT (-2)
#define SYSHAL_BLE_ERROR_LENGTH (-3)
#define SYSHAL_BLE_ERROR_FW_TYPE (-4)
#define SYSHAL_BLE_ERROR_FORBIDDEN (-5)
#define SYSHAL_BLE_ERROR_BUSY (-6)
#define SYSHAL_BLE_ERROR_DISCONNECTED (-7)
#define SYSHAL_BLE_ERROR_FAIL (-8)
#define SYSHAL_BLE_ERROR_COMMS (-100)
#define SYSHAL_BLE_ERROR_NOT_DETECTED (-101)
#define SYSHAL_BLE_ERROR_RECEIVE_PENDING (-102)
#define SYSHAL_BLE_ERROR_BUFFER_FULL (-103)
#define SYSHAL_BLE_ERROR_DEVICE (-104)
#define SYSHAL_BLE_ERROR_TRANSMIT_PENDING (-105)

typedef enum
{
    SYSHAL_BLE_EVENT_CONNECTED,
    SYSHAL_BLE_EVENT_DISCONNECTED,
    SYSHAL_BLE_EVENT_START_ADVERTISING,
    SYSHAL_BLE_EVENT_COMMAND_RECEIVED,
} syshal_ble_event_id_t;

typedef struct
{
    uint8_t buffer_size;
    uint8_t buffer[SYSHAL_BLE_MAX_BUFFER_SIZE];
    uint32_t timestamp;
} syshal_ble_event_cmd_received_t;

typedef enum
{
    SYSHAL_BLE_MODE_IDLE,
    SYSHAL_BLE_MODE_FW_UPGRADE,
    SYSHAL_BLE_MODE_BEACON,
    SYSHAL_BLE_MODE_GATT_SERVER,
    SYSHAL_BLE_MODE_GATT_CLIENT,
    SYSHAL_BLE_MODE_SCAN,
    SYSHAL_BLE_MODE_DEEP_SLEEP
} syshal_ble_mode_t;

typedef struct
{
    syshal_ble_event_id_t id;
    syshal_ble_event_cmd_received_t cmd_received;
} syshal_ble_event_t;

typedef struct
{
    sys_config_ble_settings_t *ble;
} syshal_ble_config_t;

/* Functions */

int syshal_ble_init(void);
int syshal_ble_update_config(syshal_ble_config_t ble_config);
int syshal_ble_term(void);
int syshal_ble_send_message(uint8_t *buffer,
                            size_t buffer_size);
void syshal_ble_callback(syshal_ble_event_t *event);

#endif