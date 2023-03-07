/******************************************************************************************
 * File:        an_command.cpp
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

#include "an_command.h"

uint8_t COMMAND::begin(Stream &commandPort, bool with_cmd_ack)
{
  _commandSerial = &commandPort;
  _with_cmd_ack = with_cmd_ack;

  return CMD_NO_ERROR;
}

void COMMAND::end()
{
  // Empty
}

void COMMAND::enableDebugging(Stream &debugPort,
                              bool printFullDebug)
{
  _debugSerial = &debugPort; // Grab which port the user wants us to use for debugging
  if (printFullDebug == true)
  {
    _printFullDebug = true;
  }
  _printDebug = true;
}

void COMMAND::disableDebugging(void)
{
  _printDebug = false;
  _printFullDebug = false;
}

uint8_t COMMAND::request_is_available()
{
  if (_commandSerial->available() > 0) // Say true when received a coherent frame (minimum nb bytes,...)
    return true;
  else
    return false;
}

void COMMAND::send_msg_data_packet(uint8_t data[40], uint32_t acknowledgedDate)
{
  an_packet_t an_packet;
  msg_data_packet_t msg_data_packet;

  memcpy(msg_data_packet.data, data, 40);
  msg_data_packet.acknowledgedDate = acknowledgedDate;

  encode_msg_data_packet(&an_packet, &msg_data_packet);

  an_packet_transmit(&an_packet);
}

uint8_t COMMAND::receive_msg_data_packet(uint8_t data[40])
{
  an_packet_t an_packet;
  msg_data_packet_t msg_data_packet;

  if (an_packet_receive(&an_packet, sizeof(msg_data_packet_t)) &&
      decode_msg_data_packet(&msg_data_packet, &an_packet))
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 0);
    memcpy(data, msg_data_packet.data, 40);
    return CMD_NO_ERROR;
  }
  else
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 1);
    return CMD_ERROR_AN_PACKET_NOT_VALID;
  }
}

void COMMAND::send_config_packet(uint8_t scheduler_log_data_rate,
                                 uint8_t scheduler_gnss_pvt_retry_rate,
                                 uint8_t scheduler_gnss_raw_retry_rate,
                                 uint8_t scheduler_keep_alive_rate,
                                 uint8_t scheduler_gnss_pvt_retry_count,
                                 uint8_t scheduler_gnss_raw_retry_count,
                                 uint8_t terminal_sat_search_rate,
                                 int32_t asset_latitude,
                                 int32_t asset_longitude,
                                 uint16_t asset_interface_enabled,
                                 uint8_t asset_power_saving)
{
  an_packet_t an_packet;
  config_packet_t config_packet;

  config_packet.scheduler_log_data_rate = scheduler_log_data_rate;
  config_packet.scheduler_gnss_pvt_retry_rate = scheduler_gnss_pvt_retry_rate;
  config_packet.scheduler_gnss_raw_retry_rate = scheduler_gnss_raw_retry_rate;
  config_packet.scheduler_keep_alive_rate = scheduler_keep_alive_rate;
  config_packet.scheduler_gnss_pvt_retry_count = scheduler_gnss_pvt_retry_count;
  config_packet.scheduler_gnss_raw_retry_count = scheduler_gnss_raw_retry_count;
  config_packet.terminal_sat_search_rate = terminal_sat_search_rate;
  config_packet.asset_latitude = asset_latitude;
  config_packet.asset_longitude = asset_longitude;
  config_packet.asset_interface_enabled = asset_interface_enabled;
  config_packet.asset_power_saving = asset_power_saving;

  encode_config_packet(&an_packet, &config_packet);

  an_packet_transmit(&an_packet);
}

uint8_t COMMAND::receive_config_packet(uint8_t *scheduler_log_data_rate,
                                       uint8_t *scheduler_gnss_pvt_retry_rate,
                                       uint8_t *scheduler_gnss_raw_retry_rate,
                                       uint8_t *scheduler_keep_alive_rate,
                                       uint8_t *scheduler_gnss_pvt_retry_count,
                                       uint8_t *scheduler_gnss_raw_retry_count,
                                       uint8_t *terminal_sat_search_rate,
                                       int32_t *asset_latitude,
                                       int32_t *asset_longitude,
                                       uint16_t *asset_interface_enabled,
                                       uint8_t *asset_power_saving)
{
  an_packet_t an_packet;
  config_packet_t config_packet;

  if (an_packet_receive(&an_packet, sizeof(config_packet_t)) &&
      decode_config_packet(&config_packet, &an_packet))
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 0);
    *scheduler_log_data_rate = config_packet.scheduler_log_data_rate;
    *scheduler_gnss_pvt_retry_rate = config_packet.scheduler_gnss_pvt_retry_rate;
    *scheduler_gnss_raw_retry_rate = config_packet.scheduler_gnss_raw_retry_rate;
    *scheduler_keep_alive_rate = config_packet.scheduler_keep_alive_rate;
    *scheduler_gnss_pvt_retry_count = config_packet.scheduler_gnss_pvt_retry_count;
    *scheduler_gnss_raw_retry_count = config_packet.scheduler_gnss_raw_retry_count;
    *terminal_sat_search_rate = config_packet.terminal_sat_search_rate;
    *asset_latitude = config_packet.asset_latitude;
    *asset_longitude = config_packet.asset_longitude;
    *asset_interface_enabled = config_packet.asset_interface_enabled;
    *asset_power_saving = config_packet.asset_power_saving;

    return CMD_NO_ERROR;
  }
  else
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 1);
    return CMD_ERROR_AN_PACKET_NOT_VALID;
  }
}

void COMMAND::send_cmd_data_packet(uint8_t data[40], uint32_t createdDate)
{
  an_packet_t an_packet;
  cmd_data_packet_t cmd_data_packet;

  memcpy(cmd_data_packet.data, data, 40);
  cmd_data_packet.createdDate = createdDate;

  encode_cmd_data_packet(&an_packet, &cmd_data_packet);

  an_packet_transmit(&an_packet);
}

void COMMAND::send_terminal_status_packet(uint8_t msg_in_queue,
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
                                          uint32_t loc_epoch)
{
  an_packet_t an_packet;
  terminal_status_packet_t terminal_status_packet;

  terminal_status_packet.msg_in_queue = msg_in_queue;
  terminal_status_packet.ack_msg_in_queue = ack_msg_in_queue;
  terminal_status_packet.last_rst = last_rst;
  terminal_status_packet.uptime = uptime;
  terminal_status_packet.last_mac_result = last_mac_result;
  terminal_status_packet.last_sat_search_peak_rssi = last_sat_search_peak_rssi;
  terminal_status_packet.time_since_last_sat_search = time_since_last_sat_search;
  terminal_status_packet.sys_time = sys_time;
  terminal_status_packet.v_bat = v_bat;
  terminal_status_packet.temp = temp;
  terminal_status_packet.lat = lat;
  terminal_status_packet.lon = lon;
  terminal_status_packet.loc_epoch = loc_epoch;

  encode_terminal_status_packet(&an_packet, &terminal_status_packet);

  an_packet_transmit(&an_packet);
}

void COMMAND::send_clear_msg_data_packet()
{
  an_packet_t an_packet;
  clear_msg_data_packet_t clear_msg_data_packet;

  encode_clear_msg_data_packet(&an_packet, &clear_msg_data_packet);

  an_packet_transmit(&an_packet);
}

void COMMAND::send_update_loc_data_packet()
{
  an_packet_t an_packet;
  update_loc_data_packet_t update_loc_data_packet;

  encode_update_loc_data_packet(&an_packet, &update_loc_data_packet);

  an_packet_transmit(&an_packet);
}

uint8_t COMMAND::receive_request_packet(uint8_t *id)
{
  an_packet_t an_packet;
  uint8_t id_tmp = 0; // Not really matching previous format... should create a packet for the request

  if (an_packet_receive(&an_packet, 6) &&
      decode_request_packet(&id_tmp, &an_packet))
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 0);
    memcpy(id, &id_tmp, 1);
    return CMD_NO_ERROR;
  }
  else
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 1);
    return CMD_ERROR_AN_PACKET_NOT_VALID;
  }
}

uint8_t COMMAND::an_packet_receive_id(uint8_t *an_packet_id)
{
  uint16_t rx_ctr = 0;

  while (_commandSerial->available())
  {
    rx_buffer[rx_ctr] = (uint8_t)_commandSerial->read();
    rx_ctr++;

    if (rx_ctr > AN_DECODE_MAXIMUM_FILL_SIZE)
    {
      break; // Something went wrong...
    }
  }

  if ((_printDebug == true) && (_printFullDebug == true))
  {
    _debugSerial->println("CMD: app -> asset:");
    print_array_to_hex(rx_buffer, rx_ctr);
  }

  // Clean uart buffer:
  _commandSerial->flush();

  // Quick check on data structure (data content is not yet validated)
  if (rx_buffer[2] != 0)
  {
    *an_packet_id = rx_buffer[1];
    return CMD_NO_ERROR;
  }
  else
    return CMD_ERROR_AN_PACKET_NOT_VALID;
}

void COMMAND::an_packet_transmit(an_packet_t *an_packet)
{
  an_packet_encode(an_packet);

  uint8_t buf[AN_MAXIMUM_PACKET_SIZE + AN_PACKET_HEADER_SIZE];
  memcpy(buf, an_packet->header, AN_PACKET_HEADER_SIZE);
  memcpy(&buf[AN_PACKET_HEADER_SIZE], an_packet->data, an_packet->an_length);

  if ((_printDebug == true) && (_printFullDebug == true))
  {
    _debugSerial->println("CMD: asset -> app:");
    print_array_to_hex(buf, AN_PACKET_HEADER_SIZE + an_packet->an_length);
  }

  _commandSerial->write(buf, AN_PACKET_HEADER_SIZE + an_packet->an_length);
}

bool COMMAND::an_packet_receive(an_packet_t *an_packet, uint8_t packet_length)
{
  an_decoder_t an_decoder;

  an_decoder_initialise(&an_decoder);

  memcpy(an_decoder.an_buffer, rx_buffer, AN_PACKET_HEADER_SIZE + packet_length);

  an_decoder.buffer_length = AN_PACKET_HEADER_SIZE + packet_length;

  return an_packet_decode(&an_decoder, an_packet);
}

void COMMAND::send_packet_acknowledge(an_packet_t an_packet_ack, uint8_t acknowledge_result)
{
  an_packet_t an_packet;
  acknowledge_packet_t acknowledge_packet;

  uint8_t packet_id;
  uint16_t packet_crc;

  packet_id = an_packet_ack.id;
  memcpy(&packet_crc, &an_packet_ack.header[3], sizeof(uint16_t));

  acknowledge_packet.packet_id = packet_id;
  acknowledge_packet.packet_crc = packet_crc;
  acknowledge_packet.acknowledge_result = acknowledge_result;

  encode_acknowledge_packet(&an_packet, &acknowledge_packet);

  an_packet_transmit(&an_packet);
}

void COMMAND::print_array_to_hex(uint8_t data[],
                                 size_t length)
{
  if ((_printDebug == true) && (_printFullDebug == true))
  {
    _debugSerial->println("uint8_t message[] = {");
    for (size_t i = 0; i < length; i++)
    {
      _debugSerial->print("0x");
      if ((data[i] >> 4) == 0)
      {
        _debugSerial->print("0"); // print preceeding high nibble if it's zero
      }
      _debugSerial->print(data[i], HEX);
      if (i < (length - 1))
      {
        _debugSerial->print(", ");
      }
      if ((31 - i) % 16 == 0)
      {
        _debugSerial->println();
      }
    }
    _debugSerial->println("};\n\r");
  }
}