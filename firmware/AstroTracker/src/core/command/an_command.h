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

    uint8_t receive_request_packet(uint8_t *id);

    void send_msg_data_packet(msg_data_packet_t *msg_data_packet);
    uint8_t receive_msg_data_packet(msg_data_packet_t *msg_data_packet);

    void send_config_packet(config_packet_t *config_packet);
    uint8_t receive_config_packet(config_packet_t *config_packet);

    void send_cmd_data_packet(cmd_data_packet_t *cmd_data_packet);
    uint8_t receive_cmd_data_packet(cmd_data_packet_t *cmd_data_packet);

    void send_sat_status_packet(sat_status_packet_t *sat_status_packet);
    uint8_t receive_sat_status_packet(sat_status_packet_t *sat_status_packet);

    void send_logger_status_packet(logger_status_packet_t *logger_status_packet);
    uint8_t receive_logger_status_packet(logger_status_packet_t *logger_status_packet);

    void send_gps_status_packet(gps_status_packet_t *gps_status_packet);
    uint8_t receive_gps_status_packet(gps_status_packet_t *gps_status_packet);

    void send_ble_status_packet(ble_status_packet_t *ble_status_packet);
    uint8_t receive_ble_status_packet(ble_status_packet_t *ble_status_packet);

    void send_asset_status_packet(asset_status_packet_t *asset_status_packet);
    uint8_t receive_asset_status_packet(asset_status_packet_t *asset_status_packet);

    void send_sat_bulletin_packet(sat_bulletin_packet_t *sat_bulletin_packet);
    uint8_t receive_sat_bulletin_packet(sat_bulletin_packet_t *sat_bulletin_packet);

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
