/******************************************************************************************
 * File:        syshal_sat.h
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

#ifndef _SYSHAL_SAT_h
#define _SYSHAL_SAT_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "../core/config/sys_config.h"
#include "sat/astronode.h"

#define SYSHAL_SAT_NO_ERROR (0)
#define SYSHAL_SAT_ERROR_BOOT (-1)
#define SYSHAL_SAT_ERROR_SEND_MSG (-2)
#define SYSHAL_SAT_ERROR_CLEAR_ALL_MSG (-3)
#define SYSHAL_SAT_ERROR_READ_TIME (-4)
#define SYSHAL_SAT_ERROR_READ_EVENT (-5)
#define SYSHAL_SAT_ERROR_READ_CMD (-6)
#define SYSHAL_SAT_ERROR_CLEAR_ACK_MSG (-7)
#define SYSHAL_SAT_ERROR_CLEAR_RESET (-8)
#define SYSHAL_SAT_ERROR_SET_CONFIGURATION (-9)
#define SYSHAL_SAT_ERROR_SAVE_CONFIGURATION (-10)
#define SYSHAL_SAT_ERROR_READ_NEXT_CONT_OPORT (-11)
#define SYSHAL_SAT_ERROR_SAVE_PERF_COUNTERS (-12)
#define SYSHAL_SAT_ERROR_SET_GEOLOCATION (-13)
#define SYSHAL_SAT_BUFFER_FULL (-14)
#define SYSHAL_SAT_ERROR_READ_HK (-15)
#define SYSHAL_SAT_ERROR_BUSY (-16)
#define SYSHAL_SAT_ERROR_TIMEOUT (-17)
#define SYSHAL_SAT_ERROR_DEVICE (-18)
#define SYSHAL_SAT_ERROR_INVALID_STATE (-19)
#define SYHSAL_SAT_ERROR_CLEAR_PERF_COUNTER (-20)

#define SYSHAL_SAT_BAUDRATE 9600

typedef enum
{
    SYSHAL_SAT_PWR_MODE_NONE,
    SYSHAL_SAT_PWR_MODE_ASTROCAST_EPHEMERIDES,
    SYSHAL_SAT_PWR_MODE_SAT_PASS_PREDICT,
} syshal_sat_pwr_mode_t;

typedef enum
{
    SYSHAL_SAT_EVENT_NO_EVENT = EVENT_NO_EVENT,
    SYSHAL_SAT_EVENT_MSG_ACK = EVENT_MSG_ACK,
    SYSHAL_SAT_EVENT_RESET = EVENT_RESET,
    SYSHAL_SAT_EVENT_COMMAND_RECEIVED = EVENT_CMD_RECEIVED,
    SYSHAL_SAT_EVENT_MESSAGE_PENDING = EVENT_MSG_PENDING,
    SYSHAL_SAT_EVENT_POWERED_ON,
    SYSHAL_SAT_EVENT_POWERED_OFF
} syshal_sat_event_id_t;

typedef struct
{
    uint32_t sat_detect_operation_cnt = 0;
    uint32_t signal_demod_attempt_cnt = 0;
    uint32_t ack_demod_attempt_cnt = 0;
    uint32_t sent_fragment_cnt = 0;
    uint32_t ack_fragment_cnt = 0;
    uint32_t queued_msg_cnt = 0;
    uint32_t time_start_last_contact = 0;
    uint32_t time_end_last_contact = 0;
    uint8_t peak_rssi_last_contact = 0;
    uint32_t time_peak_rssi_last_contact = 0;
    uint32_t uptime = 0;
} syshal_sat_status_t;

typedef struct
{
    uint32_t timestamp; // The timestamp of this reading
    uint16_t msg_id;
} syshal_sat_event_msg_acknowledged_t;

typedef struct
{
    uint8_t buffer_size;
    uint8_t buffer[40];
    uint32_t createdDate;
    uint32_t timestamp;
} syshal_sat_event_cmd_received_t;

typedef struct
{
    syshal_sat_event_id_t id;
    syshal_sat_event_msg_acknowledged_t msg_acknowledged;
    syshal_sat_event_cmd_received_t cmd_received;
} syshal_sat_event_t;

typedef enum
{
    SYSHAL_SAT_STATE_UNINIT,
    SYSHAL_SAT_STATE_ASLEEP,
    SYSHAL_SAT_STATE_ACTIVE,
} syshal_sat_state_t;

typedef struct
{
    sys_config_sat_settings_t *astronode;
} syshal_sat_config_t;

int syshal_sat_init(void);
int syshal_sat_update_config(syshal_sat_config_t sat_config);
int syshal_sat_term();
int syshal_sat_shutdown(void);
int syshal_sat_wake_up(void);
int syshal_sat_set_geolocation(int32_t lat,
                               int32_t lon);
int syshal_sat_send_message(uint8_t *buffer,
                            size_t buffer_size,
                            uint16_t buffer_id);
int syshal_sat_get_next_contact_oportuinty(uint32_t *delay,
                                           syshal_sat_pwr_mode_t mode,
                                           uint32_t t_now,
                                           int32_t latitude,
                                           int32_t longitude);
int syshal_sat_read_status(syshal_sat_status_t *status);
syshal_sat_state_t syshal_sat_get_state(void);
int syshal_sat_tick(void);
void syshal_sat_callback(syshal_sat_event_t *event);

#endif