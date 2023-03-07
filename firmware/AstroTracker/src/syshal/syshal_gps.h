/******************************************************************************************
 * File:        syshal_gps.h
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

#ifndef _SYSHAL_GPS_H_
#define _SYSHAL_GPS_H_

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "syshal_rtc.h"
#include "../core/config/sys_config.h"

#define SYSHAL_GPS_NO_ERROR (0)
#define SYSHAL_GPS_ERROR_BUSY (-1)
#define SYSHAL_GPS_ERROR_TIMEOUT (-2)
#define SYSHAL_GPS_ERROR_DEVICE (-3)
#define SYSHAL_GPS_ERROR_INVALID_STATE (-4)

typedef enum
{
    SYSHAL_GPS_EVENT_STATUS,
    SYSHAL_GPS_EVENT_PVT,
    SYSHAL_GPS_EVENT_RAW,
    SYSHAL_GPS_EVENT_POWERED_ON,
    SYSHAL_GPS_EVENT_POWERED_OFF
} syshal_gps_event_id_t;

typedef struct
{
    uint32_t iTOW;
    uint8_t gpsFix;
    uint8_t flags;
    uint8_t fixStat;
    uint8_t flags2;
    uint32_t ttff;
    uint32_t msss;
} syshal_gps_event_status_t;

typedef struct
{
    uint32_t timestamp;   // The timestamp of this reading
    bool timestamp_valid; // Is the timestamp given valid?
    uint32_t iTOW;        // GPS time of week of the navigation epoch
    uint8_t gpsFix;       // GPS fix information
    int32_t lon;          // Longitude
    int32_t lat;          // Latitude
    int32_t hMSL;         // Height above mean sea level
    uint32_t hAcc;        // Horizontal accuracy estimate
    uint32_t vAcc;        // Vertical accuracy estimate
    uint8_t SIV;          // Number of satellites used for the fix
    int32_t gSpeed;       // Ground speed
} syshal_gps_event_pvt_t;

typedef struct
{
    uint32_t timestamp;   // The timestamp of this reading
    uint8_t meas20[20];
} syshal_gps_event_raw_t;

typedef struct
{
    syshal_gps_event_id_t id;
    syshal_gps_event_status_t status;
    syshal_gps_event_pvt_t pvt;
    syshal_gps_event_raw_t raw;
} syshal_gps_event_t;

typedef enum
{
    SYSHAL_GPS_STATE_UNINIT,
    SYSHAL_GPS_STATE_ASLEEP,
    SYSHAL_GPS_STATE_ACQUIRING,
    SYSHAL_GPS_STATE_FIXED,
    SYSHAL_GPS_STATE_FIXED_RAW,
} syshal_gps_state_t;

typedef struct
{
    sys_config_gps_settings_t *gps;
} syshal_gps_config_t;

int syshal_gps_init(void);
int syhsal_gps_update_config(syshal_gps_config_t gps_config);
int syshal_gps_term(void);
int syshal_gps_shutdown(void);
int syshal_gps_wake_up(void);
int syshal_gps_tick(void);
syshal_gps_state_t syshal_gps_get_state(void);
void syshal_gps_callback(syshal_gps_event_t *event);

#endif