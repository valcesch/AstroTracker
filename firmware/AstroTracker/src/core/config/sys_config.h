/******************************************************************************************
 * File:        sys_config.h
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

#ifndef _SYS_CONFIG_H_
#define _SYS_CONFIG_H_

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

// Constants
#define SYS_CONFIG_NO_ERROR (0)
#define SYS_CONFIG_ERROR_INVALID_TAG (-1)
#define SYS_CONFIG_ERROR_WRONG_SIZE (-2)
#define SYS_CONFIG_ERROR_NO_MORE_TAGS (-3)
#define SYS_CONFIG_ERROR_TAG_NOT_SET (-4)
#define SYS_CONFIG_ERROR_NO_VALID_CONFIG_FILE_FOUND (-5)
#define SYS_CONFIG_ERROR_FS (-6)

#define SYS_CONFIG_TAG_ID_SIZE (sizeof(uint16_t))
#define SYS_CONFIG_TAG_DATA_SIZE(tag_type) (sizeof(((tag_type *)0)->contents))      // Size of data in tag. We exclude the set member
#define SYS_CONFIG_TAG_MAX_SIZE (SYS_CONFIG_MAX_DATA_SIZE + SYS_CONFIG_TAG_ID_SIZE) // Max size the configuration tag can be

bool sys_config_exists(uint16_t tag);
int sys_config_is_set(uint16_t tag, bool *set);
int sys_config_is_required(uint16_t tag, bool *required);
int sys_config_get(uint16_t tag, void **value);
int sys_config_set(uint16_t tag, const void *data, size_t length);
int sys_config_unset(uint16_t tag);
int sys_config_size(uint16_t tag, size_t *size);
int sys_config_iterate(uint16_t *tag, uint16_t *last_index);

enum
{
    // DEBUG
    SYS_CONFIG_TAG_DEBUG_SETTINGS,

    // BLE
    SYS_CONFIG_TAG_BLE_SETTINGS,

    // GPS
    SYS_CONFIG_TAG_GPS_SETTINGS,
    SYS_CONFIG_TAG_GPS_LOG_POSITION_ENABLE,

    // SAT
    SYS_CONFIG_TAG_SAT_SETTINGS,

    // SCHEDULER
    SYS_CONFIG_TAG_SCHEDULER_SETTINGS,

    // BATTERY
    SYS_CONFIG_TAG_BATTERY_LOG_ENABLE = 0x0900, // The battery charge state shall be enabled for logging.
    SYS_CONFIG_TAG_BATTERY_LOW_THRESHOLD,       // If set, the device will enter the low battery state and preserve any data when the battery charge goes below this threshold

    // LOGGINGS
    SYS_CONFIG_TAG_LOGGING_ENABLE,
};

typedef struct __attribute__((__packed__))
{
    bool set; // Whether or not this command tag has been set
} sys_config_hdr_t;

typedef struct __attribute__((__packed__))
{
    sys_config_hdr_t hdr;
    struct __attribute__((__packed__))
    {
    } contents;
} sys_config_debug_settings_t;

typedef struct __attribute__((__packed__))
{
    sys_config_hdr_t hdr;
    struct __attribute__((__packed__))
    {
        int8_t tx_power;
        uint16_t advert_fast_interval;
        uint16_t advert_slow_interval; // 152.5 ms, 211.25 ms, 318.75 ms, 417.5 ms, 546.25 ms, 760 ms, 852.5 ms, 1022.5 ms, 1285 ms -> Set Interval in unit of 0.625 ms
        uint16_t advert_fast_timeout;  // [s]
    } contents;
} sys_config_ble_settings_t;

typedef struct __attribute__((__packed__))
{
    sys_config_hdr_t hdr;
    struct __attribute__((__packed__))
    {
        bool with_gps;
        bool with_galileo;
        bool with_beidou;
        bool with_glonass;
        bool with_rxm_meas20;
        uint16_t raw_timeout_s;
    } contents;
} sys_config_gps_settings_t;

typedef struct __attribute__((__packed__))
{
    sys_config_hdr_t hdr;
    struct __attribute__((__packed__))
    {
        bool with_pld_ack;
        bool with_geo_loc;
        bool with_ephemeris;
        bool with_deep_sleep_en;
        bool with_msg_ack_pin_en;
        bool with_msg_reset_pin_en;
        bool with_cmd_event_pin_en;
        bool with_tx_pend_event_pin_en;
        uint8_t sat_search_rate;
        bool sat_force_search;
    } contents;
} sys_config_sat_settings_t;

typedef struct __attribute__((__packed__))
{
    sys_config_hdr_t hdr;
    struct __attribute__((__packed__))
    {
        uint8_t gps_interval_h;
        uint8_t gps_timeout_s;
    } contents;
} sys_config_scheduler_settings_t;

typedef struct __attribute__((__packed__))
{
    sys_config_hdr_t hdr;
    struct __attribute__((__packed__))
    {
        uint8_t threshold;
    } contents;
} sys_config_battery_low_threshold_t;

typedef struct __attribute__((__packed__))
{
    sys_config_hdr_t hdr;
    struct __attribute__((__packed__))
    {
        uint8_t enable;
    } contents;
} sys_config_gps_log_position_enable_t;

typedef struct __attribute__((__packed__))
{
    sys_config_hdr_t hdr;
    struct __attribute__((__packed__))
    {
        uint8_t enable;
    } contents;
} sys_config_logging_enable_t;

typedef struct
{
    uint8_t format_version; // A version number to keep track of the format/contents of this struct
    sys_config_debug_settings_t debug_settings;
    sys_config_ble_settings_t ble_settings;
    sys_config_gps_settings_t gps_settings;
    sys_config_sat_settings_t sat_settings;
    sys_config_scheduler_settings_t scheduler_settings;
    sys_config_battery_low_threshold_t battery_low_threshold;
    sys_config_gps_log_position_enable_t gps_log_position_enable;
    sys_config_logging_enable_t logging_enable;
} sys_config_t;

extern sys_config_t sys_config;

#endif /* _SYS_CONFIG_H_ */