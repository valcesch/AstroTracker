/******************************************************************************************
 * File:        an_command.h
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

#ifndef _AN_COMMAND_H
#define _AN_COMMAND_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "an_packet_protocol.h"
#include "an_packets.h"

typedef enum
{
    CMD_NO_ERROR = 0,
    CMD_ERROR_AN_PACKET_NOT_VALID,
} cmd_status_e;

class COMMAND
{
public:
    uint8_t begin(Stream &commandPort, bool with_cmd_ack);
    void end();

    uint8_t request_is_available();
    uint8_t an_packet_receive_id(uint8_t *an_packet_id);

    // send - TODO
    uint8_t receive_request_packet(uint8_t *id);

    void send_msg_data_packet(uint8_t data[40],
                              uint32_t acknowledgedDate);
    uint8_t receive_msg_data_packet(uint8_t data[40]);

    void send_config_packet(uint8_t scheduler_log_data_rate,
                            uint8_t scheduler_gnss_pvt_retry_rate,
                            uint8_t scheduler_gnss_raw_retry_rate,
                            uint8_t scheduler_keep_alive_rate,
                            uint8_t scheduler_gnss_pvt_retry_count,
                            uint8_t scheduler_gnss_raw_retry_count,
                            uint8_t terminal_sat_search_rate,
                            int32_t asset_latitude,
                            int32_t asset_longitude,
                            uint16_t asset_interface_enabled,
                            uint8_t asset_power_saving);
    uint8_t receive_config_packet(uint8_t *scheduler_log_data_rate,
                                  uint8_t *scheduler_gnss_pvt_retry_rate,
                                  uint8_t *scheduler_gnss_raw_retry_rate,
                                  uint8_t *scheduler_keep_alive_rate,
                                  uint8_t *scheduler_gnss_pvt_retry_count,
                                  uint8_t *scheduler_gnss_raw_retry_count,
                                  uint8_t *terminal_sat_search_rate,
                                  int32_t *asset_latitude,
                                  int32_t *asset_longitude,
                                  uint16_t *asset_interface_enabled,
                                  uint8_t *asset_power_saving);

    void send_cmd_data_packet(uint8_t data[40],
                              uint32_t createdDate);
    // receive - TODO

    void send_terminal_status_packet(uint8_t msg_in_queue,
                                     uint8_t ack_msg_in_queue,
                                     uint8_t last_rst,
                                     uint32_t uptime,
                                     uint8_t last_mac_result,
                                     uint8_t last_sat_search_peak_rssi,
                                     uint32_t time_since_last_sat_search,
                                     uint32_t sys_time,
                                     uint8_t v_bat,
                                     int8_t temp,
                                     int32_t lat,
                                     int32_t lon,
                                     uint32_t loc_epoch);
    // receive - TODO

    void send_clear_msg_data_packet();
    // receive - TODO

    void send_update_loc_data_packet();
    // receive - TODO

    void enableDebugging(Stream &debugPort,
                         bool printFullDebug);
    void disableDebugging(void);

private:
    Stream *_commandSerial;
    Stream *_debugSerial;

    bool _printDebug = false;     // Flag to print the serial commands we are sending to the Serial port for debug
    bool _printFullDebug = false; // Flag to print full debug messages. Useful for UART debugging
    bool _with_cmd_ack = true;    // Enable, disable commands acknowledge

    uint8_t rx_buffer[AN_DECODE_MAXIMUM_FILL_SIZE] = {0};

    void an_packet_transmit(an_packet_t *an_packet);
    bool an_packet_receive(an_packet_t *an_packet,
                           uint8_t packet_length);

    void send_packet_acknowledge(an_packet_t an_packet_ack,
                                 uint8_t acknowledge_result);

    void print_array_to_hex(uint8_t data[],
                            size_t length);
};

#endif
