/******************************************************************************************
 * File:        an_packets.h
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

#ifndef _AN_PACKETS_H
#define _AN_PACKETS_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "an_packet_protocol.h"

typedef enum
{
    packet_id_acknowledge,
    packet_id_request,
    packet_id_msg_data,
    packet_id_sat_status,
    packet_id_logger_status,
    packet_id_gps_status,
    packet_id_ble_status,
    packet_id_asset_status,
    packet_id_cmd_data,
    packet_id_clear_msg_data,
    packet_id_update_loc_data,
    packet_id_config,
    packet_id_geoloc,
    packet_id_sat_bulletin,
} packet_id_e;

static const char *packet_id_str[] =
    {
        [packet_id_acknowledge] = "AN_PACKET_ACK",
        [packet_id_request] = "AN_PAQUET_REQUEST",
        [packet_id_msg_data] = "AN_PACKET_MSG_DATA",
        [packet_id_sat_status] = "AN_PACKET_SAT_STATUS",
        [packet_id_logger_status] = "AN_PACKET_LOGGER_STATUS",
        [packet_id_gps_status] = "AN_PACKET_GPS_STATUS",
        [packet_id_ble_status] = "AN_PACKET_BLE_STATUS",
        [packet_id_asset_status] = "AN_PACKET_ASSET_STATUS",
        [packet_id_cmd_data] = "AN_PACKET_CMD_DATA",
        [packet_id_clear_msg_data] = "AN_PACKET_CLEAR_MSG_DATA",
        [packet_id_update_loc_data] = "AN_PACKET_UPDATE_LOC_DATA",
        [packet_id_config] = "AN_PACKET_CONFIG",
        [packet_id_geoloc] = "AN_PACKET_GEOLOC",
        [packet_id_sat_bulletin] = "AN_PACKET_SAT_BULLETIN",
};

typedef enum
{
    acknowledge_success,
    acknowledge_failure_crc,
    acknowledge_failure_length,
    acknowledge_failure_range,
    acknowledge_failure_flash,
    acknowledge_failure_not_ready,
    acknowledge_failure_unknown_packet
} acknowledge_result_e;

typedef struct __attribute__((__packed__))
{
    uint8_t packet_id;
    uint16_t packet_crc;
    uint8_t acknowledge_result;
} acknowledge_packet_t;

typedef struct __attribute__((__packed__))
{
    uint8_t data[40];
    uint32_t acknowledgedDate;
} msg_data_packet_t;

typedef struct __attribute__((__packed__))
{
    uint8_t data[40];
    uint32_t createdDate;
} cmd_data_packet_t;

typedef struct __attribute__((__packed__))
{
    uint32_t sat_detect_operation_cnt;
    uint32_t signal_demod_attempt_cnt;
    uint32_t ack_demod_attempt_cnt;
    uint32_t sent_fragment_cnt;
    uint32_t ack_fragment_cnt;
    uint32_t queued_msg_cnt;
    uint32_t time_start_last_contact;
    uint32_t time_end_last_contact;
    uint8_t peak_rssi_last_contact;
    uint32_t time_peak_rssi_last_contact;
    uint16_t reset_cnt;
    uint32_t uptime;
} sat_status_packet_t;

typedef struct __attribute__((__packed__))
{
    uint16_t u_msg_cnt;
    uint16_t u_cmd_cnt;
    uint16_t pvt_cnt;
    uint16_t raw_cnt;
    // uint32_t total_mem_size;
} logger_status_packet_t;

typedef struct __attribute__((__packed__))
{
    uint16_t meas_cnt;
    int32_t last_loc_lat;
    int32_t last_loc_lon;
    uint32_t time_last_update;
    uint32_t uptime;
} gps_status_packet_t;

typedef struct __attribute__((__packed__))
{
    uint32_t advert_burst_cnt;
    uint16_t user_connection_cnt;
} ble_status_packet_t;

typedef struct __attribute__((__packed__))
{
    uint32_t up_time_ms;
    uint32_t sys_time;
} asset_status_packet_t;

typedef struct __attribute__((__packed__))
{
    // GPS config
    bool gps_settings_with_gps;
    bool gps_settings_with_galileo;
    bool gps_settings_with_beidou;
    bool gps_settings_with_glonass;
    bool gps_settings_with_rxm_meas20;
    uint8_t gps_settings_raw_timeout_s;

    // sat modem config
    bool sat_settings_with_pld_ack;
    bool sat_settings_with_geo_loc;
    bool sat_settings_with_ephemeris;
    bool sat_settings_with_deep_sleep_en;
    bool sat_settings_with_msg_ack_pin_en;
    bool sat_settings_with_msg_reset_pin_en;
    bool sat_settings_with_cmd_event_pin_en;
    bool sat_settings_with_tx_pend_event_pin_en;
    bool sat_settings_sat_force_search;
    uint8_t sat_settings_sat_search_rate;

    // Scheduler config
    uint8_t scheduler_settings_gps_interval_h;
    uint8_t gps_settings_pvt_timeout_s;

    // Asset status
    uint8_t battery_low_threshhold;

} config_packet_t;

typedef struct __attribute__((__packed__))
{
    // empty
} clear_msg_data_packet_t;

typedef struct __attribute__((__packed__))
{
    // empty
} update_loc_data_packet_t;

typedef struct __attribute__((__packed__))
{
    uint8_t plane_id;
    uint32_t t_expir;
} sat_bulletin_packet_t;

void encode_acknowledge_packet(an_packet_t *an_packet, acknowledge_packet_t *acknowledge_packet);
uint8_t decode_acknowledge_packet(acknowledge_packet_t *acknowledge_packet, an_packet_t *an_packet);

void encode_cmd_data_packet(an_packet_t *an_packet, cmd_data_packet_t *cmd_data_packet);
uint8_t decode_cmd_data_packet(cmd_data_packet_t *cmd_data_packet, an_packet_t *an_packet);

void encode_msg_data_packet(an_packet_t *an_packet, msg_data_packet_t *msg_data_packet);
uint8_t decode_msg_data_packet(msg_data_packet_t *msg_data_packet, an_packet_t *an_packet);

void encode_sat_status_packet(an_packet_t *an_packet, sat_status_packet_t *sat_status_packet);
uint8_t decode_sat_status_packet(sat_status_packet_t *read_sat_status_packet, an_packet_t *an_packet);

void encode_logger_status_packet(an_packet_t *an_packet, logger_status_packet_t *logger_status_packet);
uint8_t decode_logger_status_packet(logger_status_packet_t *logger_status_packet, an_packet_t *an_packet);

void encode_gps_status_packet(an_packet_t *an_packet, gps_status_packet_t *gps_status_packet);
uint8_t decode_gps_status_packet(gps_status_packet_t *gps_status_packet, an_packet_t *an_packet);

void encode_ble_status_packet(an_packet_t *an_packet, ble_status_packet_t *ble_status_packet);
uint8_t decode_ble_status_packet(ble_status_packet_t *ble_status_packet, an_packet_t *an_packet);

void encode_asset_status_packet(an_packet_t *an_packet, asset_status_packet_t *asset_status_packet);
uint8_t decode_asset_status_packet(asset_status_packet_t *asset_status_packet, an_packet_t *an_packet);

void encode_sat_bulletin_packet(an_packet_t *an_packet, sat_bulletin_packet_t *sat_bulletin_packet);
uint8_t decode_sat_bulletin_packet(sat_bulletin_packet_t *sat_bulletin_packet, an_packet_t *an_packet);

void encode_config_packet(an_packet_t *an_packet, config_packet_t *config_packet);
uint8_t decode_config_packet(config_packet_t *config_packet, an_packet_t *an_packet);

void encode_request_packet(an_packet_t *an_packet, uint8_t requested_packet_id);
uint8_t decode_request_packet(uint8_t *id, an_packet_t *an_packet);

#endif
