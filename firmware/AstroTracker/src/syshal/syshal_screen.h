/******************************************************************************************
 * File:        syshal_screen.h
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

#ifndef _SYSHAL_SCREEN_H_
#define _SYSHAL_SCREEN_H_

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "../core/config/sys_config.h"

#define SYSHAL_SCREEN_NO_ERROR (0)
#define SYSHAL_SCREEN_ERROR_INVALID_STATE (-1)
#define SYSHAL_SCREEN_ERROR_DEVICE (-2)

typedef struct __attribute__((__packed__))
{
    uint8_t v_bat;
    int8_t temp;
    uint16_t u_msg_cnt;
    uint16_t u_cmd_cnt;
    uint16_t pvt_cnt;
    int32_t last_loc_lat;
    int32_t last_loc_lon;
} syshal_screen_status_t;

typedef enum
{
    SYSHAL_SCREEN_MENU_MODE_HOME_PAGE = -1,
    SYSHAL_SCREEN_MENU_MODE_STATUS_PAGE,
    SYSHAL_SCREEN_MENU_MODE_SHUTDOWN_PAGE,
    SYSHAL_SCREEN_MENU_MODE_TX_MSG_LST_PAGE,
    SYSHAL_SCREEN_MENU_MODE_RX_MSG_LST_PAGE,
    SYSHAL_SCREEN_MENU_MODE_PRESET_MSG_1,
    SYSHAL_SCREEN_MENU_MODE_PRESET_MSG_2,
    SYSHAL_SCREEN_MENU_MODE_PRESET_MSG_3,
    SYSHAL_SCREEN_MENU_MODE_PRESET_MSG_4,
    SYSHAL_SCREEN_MENU_MODE_PRESET_MSG_5,
    SYSHAL_SCREEN_MENU_MODE_UPDATE_GEOLOC_PAGE,
    SYSHAL_SCREEN_MENU_MODE_CLEAR_ALL_USER_MSG_PAGE,
    SYSHAL_SCREEN_MENU_MODE_FIRMWARE_INFO_PAGE,
} syshal_screen_menu_mode_t;

typedef enum
{
    SYSHAL_SCREEN_PRESET_MSG_1,
    SYSHAL_SCREEN_PRESET_MSG_2,
    SYSHAL_SCREEN_PRESET_MSG_3,
    SYSHAL_SCREEN_PRESET_MSG_4,
    SYSHAL_SCREEN_PRESET_MSG_5,
} syshal_screen_preset_msg_id_t;

typedef struct __attribute__((__packed__))
{
    uint8_t buffer_size;
    uint8_t buffer[40];
    uint32_t timestamp;
} syshal_screen_event_preset_msg_t;

typedef enum
{
    SYSHAL_SCREEN_EVENT_SHUTDOWN,
    SYSHAL_SCREEN_EVENT_PRESET_MSG,
    SYSHAL_SCREEN_EVENT_UPDATE_GEOLOC,
    SYSHAL_SCREEN_EVENT_CLEAR_ALL_USER_MSG,
    SYSHAL_SCREEN_EVENT_UPDATE_STATUS_REQUEST,
    SYSHAL_SCREEN_EVENT_DISPLAY_ON,
    SYSHAL_SCREEN_EVENT_DISPLAY_OFF,
    SYSHAL_SCREEN_EVENT_BUTTON_PRESSED,
} syshal_screen_event_id_t;

typedef struct __attribute__((__packed__))
{
    syshal_screen_event_id_t id;
    syshal_screen_event_preset_msg_t preset_msg;
} syshal_screen_event_t;

typedef enum
{
    SYSHAL_SCREEN_STATE_UNINIT,
    SYSHAL_SCREEN_STATE_ASLEEP,
    SYSHAL_SCREEN_STATE_DISPLAYING,
} syshal_screen_state_t;

typedef struct
{
    sys_config_screen_settings_t *screen;
} syshal_screen_config_t;

int syshal_screen_init(void);
int syshal_screen_update_config(syshal_screen_config_t screen_config);
int syshal_screen_term(void);
int syshal_screen_shutdown(void);
int syshal_screen_wake_up(void);
void syshal_screen_set_status(syshal_screen_status_t screen_status);
syshal_screen_state_t syshal_screen_get_state(void);
int syshal_screen_tick(void);
void syshal_screen_callback(syshal_screen_event_t *event);

#endif