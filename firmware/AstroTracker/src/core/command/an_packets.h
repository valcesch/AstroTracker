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
    packet_id_terminal_status,
    packet_id_cmd_data,
    packet_id_clear_msg_data,
    packet_id_update_loc_data,
    packet_id_config,
    packet_id_geoloc,
} packet_id_e;

static const char *packet_id_str[] =
    {
        [packet_id_acknowledge] = "AN_PACKET_ACK",
        [packet_id_request] = "AN_PAQUET_REQUEST",
        [packet_id_msg_data] = "AN_PACKET_MSG_DATA",
        [packet_id_terminal_status] = "AN_PACKET_STATUS",
        [packet_id_cmd_data] = "AN_PACKET_CMD_DATA",
        [packet_id_clear_msg_data] = "AN_PACKET_CLEAR_MSG_DATA",
        [packet_id_update_loc_data] = "AN_PACKET_UPDATE_LOC_DATA",
        [packet_id_config] = "AN_PACKET_CONFIG",
        [packet_id_geoloc] = "AN_PACKET_GEOLOC",
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

typedef struct
{
    uint8_t packet_id;
    uint16_t packet_crc;
    uint8_t acknowledge_result;
} acknowledge_packet_t;

typedef struct
{
    uint8_t data[40];
    uint32_t acknowledgedDate;
} msg_data_packet_t;

typedef struct
{
    uint8_t data[40];
    uint32_t createdDate;
} cmd_data_packet_t;

typedef struct
{
    // Module state
    uint8_t msg_in_queue;
    uint8_t ack_msg_in_queue;
    uint8_t last_rst;
    uint32_t uptime;

    // Environmental details
    uint8_t last_mac_result;
    uint8_t last_sat_search_peak_rssi;
    uint32_t time_since_last_sat_search;

    // Tracker vitals
    uint32_t sys_time;
    uint8_t v_bat;
    int8_t temp;

    // Geolocation
    int32_t lat;
    int32_t lon;
    uint32_t loc_epoch;

} terminal_status_packet_t;

typedef struct
{
    uint8_t scheduler_log_data_rate;
    uint8_t scheduler_gnss_pvt_retry_rate;
    uint8_t scheduler_gnss_raw_retry_rate;
    uint8_t scheduler_keep_alive_rate;
    uint8_t scheduler_gnss_pvt_retry_count;
    uint8_t scheduler_gnss_raw_retry_count;
    uint8_t terminal_sat_search_rate;
    int32_t asset_latitude;
    int32_t asset_longitude;
    uint16_t asset_interface_enabled;
    uint8_t asset_power_saving;
} config_packet_t;

typedef struct
{
    // empty
} clear_msg_data_packet_t;

typedef struct
{
    // empty
} update_loc_data_packet_t;

void encode_acknowledge_packet(an_packet_t *an_packet, acknowledge_packet_t *acknowledge_packet);
uint8_t decode_acknowledge_packet(acknowledge_packet_t *acknowledge_packet, an_packet_t *an_packet);

void encode_cmd_data_packet(an_packet_t *an_packet, cmd_data_packet_t *cmd_data_packet);
uint8_t decode_cmd_data_packet(cmd_data_packet_t *cmd_data_packet, an_packet_t *an_packet);

void encode_msg_data_packet(an_packet_t *an_packet, msg_data_packet_t *msg_data_packet);
uint8_t decode_msg_data_packet(msg_data_packet_t *msg_data_packet, an_packet_t *an_packet);

void encode_terminal_status_packet(an_packet_t *an_packet, terminal_status_packet_t *terminal_status_packet);
uint8_t decode_terminal_status_packet(terminal_status_packet_t *read_terminal_status_packet, an_packet_t *an_packet);

void encode_clear_msg_data_packet(an_packet_t *an_packet, clear_msg_data_packet_t *clear_msg_data_packet);
uint8_t decode_clear_msg_data_packet(clear_msg_data_packet_t *clear_msg_data_packet, an_packet_t *an_packet);

void encode_update_loc_data_packet(an_packet_t *an_packet, update_loc_data_packet_t *update_loc_data_packet);
uint8_t decode_update_loc_data_packet(update_loc_data_packet_t *update_loc_data_packet, an_packet_t *an_packet);

void encode_config_packet(an_packet_t *an_packet, config_packet_t *config_packet);
uint8_t decode_config_packet(config_packet_t *config_packet, an_packet_t *an_packet);

void encode_request_packet(an_packet_t *an_packet, uint8_t requested_packet_id);
uint8_t decode_request_packet(uint8_t *id, an_packet_t *an_packet);

#endif
