/******************************************************************************************
 * File:        scheduler.cpp
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

#ifndef _SCHEDULER_h
#define _SCHEDULER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "../config/sys_config.h"

#define SCHEDULER_NO_ERROR (0)
#define SCHEDULER_ERROR_INVALID_STATE (-1)
#define SCHEDULER_ERROR_INVALID_PARAM (-2)

typedef enum
{
    SCHEDULER_EVENT_GPS_START,
    SCHEDULER_EVENT_SATPASS_START,
} scheduler_event_id_t;

typedef struct
{
    scheduler_event_id_t id;
} scheduler_event_t;

typedef struct
{
    sys_config_gps_scheduler_settings_t *scheduler;
} scheduler_gps_config_t;

typedef struct
{
    sys_config_satpass_scheduler_settings_t *scheduler;
} scheduler_satpass_config_t;

int scheduler_init(void);
int scheduler_term(void);
int scheduler_get_timestamp_next_alarm(uint32_t *timestamp);

int scheduler_gps_update_config(scheduler_gps_config_t scheduler_gps_config);
int scheduler_gps_start(void);
int scheduler_gps_stop(void);

int scheduler_satpass_update_config(scheduler_satpass_config_t scheduler_satpass_config);
int scheduler_satpass_start(void);
int scheduler_satpass_stop(void);

int scheduler_tick(void);

void scheduler_gps_callback(scheduler_event_t *event);
void scheduler_satpass_callback(scheduler_event_t *event);

/* Hints from UBLOX on GNSS configuration and fallback strategy
https://developer.thingstream.io/guides/location-services/cloudlocate-getting-started/mixed_mode
If during your testing phase you are in a very good sky visibility condition, you can set the Tc
parameter to 1 (second)  so that the fallback condition is immediately triggered and Tx to 40
(seconds) so that the GNSS has enough time to estimate a position locally (without using
assistance data).
In case you are in challenging signal conditions, we suggest you to set Tc to 10 seconds and Tx
to 120 seconds or lower if this is not applicable for your use case.
*/

#endif