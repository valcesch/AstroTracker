/******************************************************************************************
 * File:        an_packets.cpp
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

#include "an_packets.h"

void encode_acknowledge_packet(an_packet_t *an_packet, acknowledge_packet_t *acknowledge_packet)
{
  an_packet->id = packet_id_acknowledge;
  an_packet->an_length = sizeof(acknowledge_packet_t);
  memcpy(&an_packet->data[0], &acknowledge_packet->packet_id, 1 * sizeof(uint8_t));
  memcpy(&an_packet->data[1], &acknowledge_packet->packet_crc, 1 * sizeof(uint16_t));
  memcpy(&an_packet->data[3], &acknowledge_packet->acknowledge_result, 1 * sizeof(uint8_t));
}

uint8_t decode_acknowledge_packet(acknowledge_packet_t *acknowledge_packet, an_packet_t *an_packet)
{
  uint8_t packet_decoded = false;
  if (an_packet->id == packet_id_acknowledge && an_packet->an_length == sizeof(acknowledge_packet_t))
  {
    memcpy(&acknowledge_packet->packet_id, &an_packet->data[0], 1 * sizeof(uint8_t));
    memcpy(&acknowledge_packet->packet_crc, &an_packet->data[1], sizeof(uint16_t));
    memcpy(&acknowledge_packet->acknowledge_result, &an_packet->data[3], 1 * sizeof(uint8_t));
    packet_decoded = true;
  }
  return packet_decoded;
}

void encode_cmd_data_packet(an_packet_t *an_packet, cmd_data_packet_t *cmd_data_packet)
{
  an_packet->id = packet_id_cmd_data;
  an_packet->an_length = sizeof(cmd_data_packet_t);
  memcpy(&an_packet->data[0], &cmd_data_packet->data, 40 * sizeof(uint8_t));
  memcpy(&an_packet->data[40], &cmd_data_packet->createdDate, 1 * sizeof(uint32_t));
}

uint8_t decode_cmd_data_packet(cmd_data_packet_t *cmd_data_packet, an_packet_t *an_packet)
{
  uint8_t packet_decoded = false;
  if (an_packet->id == packet_id_cmd_data && an_packet->an_length == sizeof(cmd_data_packet_t))
  {
    memcpy(&cmd_data_packet->data, &an_packet->data[0], 40 * sizeof(uint8_t));
    memcpy(&cmd_data_packet->createdDate, &an_packet->data[40], 1 * sizeof(uint32_t));
    packet_decoded = true;
  }
  return packet_decoded;
}

void encode_msg_data_packet(an_packet_t *an_packet, msg_data_packet_t *msg_data_packet)
{
  an_packet->id = packet_id_msg_data;
  an_packet->an_length = sizeof(msg_data_packet_t);
  memcpy(&an_packet->data[0], &msg_data_packet->data, 40 * sizeof(uint8_t));
  memcpy(&an_packet->data[40], &msg_data_packet->acknowledgedDate, 1 * sizeof(uint32_t));
}

uint8_t decode_msg_data_packet(msg_data_packet_t *msg_data_packet, an_packet_t *an_packet)
{
  uint8_t packet_decoded = false;
  if (an_packet->id == packet_id_msg_data && an_packet->an_length == sizeof(msg_data_packet_t))
  {
    memcpy(&msg_data_packet->data, &an_packet->data[0], 40 * sizeof(uint8_t));
    memcpy(&msg_data_packet->acknowledgedDate, &an_packet->data[40], 1 * sizeof(uint32_t));
    packet_decoded = true;
  }
  return packet_decoded;
}

void encode_terminal_status_packet(an_packet_t *an_packet, terminal_status_packet_t *terminal_status_packet)
{
  an_packet->id = packet_id_terminal_status;
  an_packet->an_length = sizeof(terminal_status_packet_t);
  memcpy(&an_packet->data[0], &terminal_status_packet->msg_in_queue, 1 * sizeof(uint8_t));
  memcpy(&an_packet->data[1], &terminal_status_packet->ack_msg_in_queue, 1 * sizeof(uint8_t));
  memcpy(&an_packet->data[2], &terminal_status_packet->last_rst, 1 * sizeof(uint8_t));
  memcpy(&an_packet->data[3], &terminal_status_packet->uptime, 1 * sizeof(uint32_t));
  memcpy(&an_packet->data[7], &terminal_status_packet->last_mac_result, 1 * sizeof(uint8_t));
  memcpy(&an_packet->data[8], &terminal_status_packet->last_sat_search_peak_rssi, 1 * sizeof(uint8_t));
  memcpy(&an_packet->data[9], &terminal_status_packet->time_since_last_sat_search, 1 * sizeof(uint32_t));
  memcpy(&an_packet->data[13], &terminal_status_packet->sys_time, 1 * sizeof(uint32_t));
  memcpy(&an_packet->data[17], &terminal_status_packet->v_bat, 1 * sizeof(uint8_t));
  memcpy(&an_packet->data[18], &terminal_status_packet->temp, 1 * sizeof(int8_t));
  memcpy(&an_packet->data[19], &terminal_status_packet->lat, 1 * sizeof(int32_t));
  memcpy(&an_packet->data[23], &terminal_status_packet->lon, 1 * sizeof(int32_t));
  memcpy(&an_packet->data[27], &terminal_status_packet->loc_epoch, 1 * sizeof(uint32_t));
}

uint8_t decode_terminal_status_packet(terminal_status_packet_t *terminal_status_packet, an_packet_t *an_packet)
{
  uint8_t packet_decoded = false;
  if (an_packet->id == packet_id_terminal_status && an_packet->an_length == sizeof(terminal_status_packet_t))
  {
    memcpy(&terminal_status_packet->msg_in_queue, &an_packet->data[0], 1 * sizeof(uint8_t));
    memcpy(&terminal_status_packet->ack_msg_in_queue, &an_packet->data[1], 1 * sizeof(uint8_t));
    memcpy(&terminal_status_packet->last_rst, &an_packet->data[2], 1 * sizeof(uint8_t));
    memcpy(&terminal_status_packet->uptime, &an_packet->data[3], 1 * sizeof(uint32_t));
    memcpy(&terminal_status_packet->last_mac_result, &an_packet->data[7], 1 * sizeof(uint8_t));
    memcpy(&terminal_status_packet->last_sat_search_peak_rssi, &an_packet->data[8], 1 * sizeof(uint8_t));
    memcpy(&terminal_status_packet->time_since_last_sat_search, &an_packet->data[9], 1 * sizeof(uint32_t));
    memcpy(&terminal_status_packet->sys_time, &an_packet->data[13], 1 * sizeof(uint32_t));
    memcpy(&terminal_status_packet->v_bat, &an_packet->data[17], 1 * sizeof(uint8_t));
    memcpy(&terminal_status_packet->temp, &an_packet->data[18], 1 * sizeof(int8_t));
    memcpy(&terminal_status_packet->lat, &an_packet->data[19], 1 * sizeof(int32_t));
    memcpy(&terminal_status_packet->lon, &an_packet->data[23], 1 * sizeof(int32_t));
    memcpy(&terminal_status_packet->loc_epoch, &an_packet->data[27], 1 * sizeof(uint32_t));
    packet_decoded = true;
  }
  return packet_decoded;
}

void encode_clear_msg_data_packet(an_packet_t *an_packet, clear_msg_data_packet_t *clear_msg_data_packet)
{
  an_packet->id = packet_id_clear_msg_data;
  an_packet->an_length = sizeof(clear_msg_data_packet_t);
  // Empty
}

uint8_t decode_clear_msg_data_packet(clear_msg_data_packet_t *clear_msg_data_packet, an_packet_t *an_packet)
{
  uint8_t packet_decoded = false;
  if (an_packet->id == packet_id_clear_msg_data && an_packet->an_length == sizeof(clear_msg_data_packet_t))
  {
    // Empty
    packet_decoded = true;
  }
  return packet_decoded;
}


void encode_update_loc_data_packet(an_packet_t *an_packet, update_loc_data_packet_t *update_loc_data_packet)
{
  an_packet->id = packet_id_update_loc_data;
  an_packet->an_length = sizeof(update_loc_data_packet_t);
  // Empty
}

uint8_t decode_update_loc_data_packet(update_loc_data_packet_t *update_loc_data_packet, an_packet_t *an_packet)
{
  uint8_t packet_decoded = false;
  if (an_packet->id == packet_id_update_loc_data && an_packet->an_length == sizeof(update_loc_data_packet_t))
  {
    // Empty
    packet_decoded = true;
  }
  return packet_decoded;
}

void encode_config_packet(an_packet_t *an_packet, config_packet_t *config_packet)
{
  an_packet->id = packet_id_config;
  an_packet->an_length = sizeof(config_packet_t);
  memcpy(&an_packet->data[0], &config_packet->scheduler_log_data_rate, 1 * sizeof(uint8_t));
  memcpy(&an_packet->data[1], &config_packet->scheduler_gnss_pvt_retry_rate, 1 * sizeof(uint8_t));
  memcpy(&an_packet->data[2], &config_packet->scheduler_gnss_raw_retry_rate, 1 * sizeof(uint8_t));
  memcpy(&an_packet->data[3], &config_packet->scheduler_keep_alive_rate, 1 * sizeof(uint8_t));
  memcpy(&an_packet->data[4], &config_packet->scheduler_gnss_pvt_retry_count, 1 * sizeof(uint8_t));
  memcpy(&an_packet->data[5], &config_packet->scheduler_gnss_raw_retry_count, 1 * sizeof(uint8_t));
  memcpy(&an_packet->data[6], &config_packet->terminal_sat_search_rate, 1 * sizeof(uint8_t));
  memcpy(&an_packet->data[7], &config_packet->asset_latitude, 1 * sizeof(int32_t));
  memcpy(&an_packet->data[11], &config_packet->asset_longitude, 1 * sizeof(int32_t));
  memcpy(&an_packet->data[15], &config_packet->asset_interface_enabled, 1 * sizeof(uint16_t));
  memcpy(&an_packet->data[17], &config_packet->asset_power_saving, 1 * sizeof(uint8_t));
}

uint8_t decode_config_packet(config_packet_t *config_packet, an_packet_t *an_packet)
{
  uint8_t packet_decoded = false;
  if (an_packet->id == packet_id_config && an_packet->an_length == sizeof(config_packet_t))
  {
    memcpy(&config_packet->scheduler_log_data_rate, &an_packet->data[0], 1 * sizeof(uint8_t));
    memcpy(&config_packet->scheduler_gnss_pvt_retry_rate, &an_packet->data[1], 1 * sizeof(uint8_t));
    memcpy(&config_packet->scheduler_gnss_raw_retry_rate, &an_packet->data[2], 1 * sizeof(uint8_t));
    memcpy(&config_packet->scheduler_keep_alive_rate, &an_packet->data[3], 1 * sizeof(uint8_t));
    memcpy(&config_packet->scheduler_gnss_pvt_retry_count, &an_packet->data[4], 1 * sizeof(uint8_t));
    memcpy(&config_packet->scheduler_gnss_raw_retry_count, &an_packet->data[5], 1 * sizeof(uint8_t));
    memcpy(&config_packet->terminal_sat_search_rate, &an_packet->data[6], 1 * sizeof(uint8_t));
    memcpy(&config_packet->asset_latitude, &an_packet->data[7], 1 * sizeof(int32_t));
    memcpy(&config_packet->asset_longitude, &an_packet->data[11], 1 * sizeof(int32_t));
    memcpy(&config_packet->asset_interface_enabled, &an_packet->data[15], 1 * sizeof(uint16_t));
    memcpy(&config_packet->asset_power_saving, &an_packet->data[17], 1 * sizeof(uint8_t));
    packet_decoded = true;
  }
  return packet_decoded;
}

void encode_request_packet(an_packet_t *an_packet, uint8_t requested_packet_id)
{
  an_packet->id = packet_id_request;
  an_packet->an_length = 1;
  an_packet->data[0] = requested_packet_id;
}

uint8_t decode_request_packet(uint8_t *id, an_packet_t *an_packet)
{
  uint8_t packet_decoded = false;
  if (an_packet->id == packet_id_request && an_packet->an_length == sizeof(uint8_t))
  {
    memcpy(id, &an_packet->data[0], 1 * sizeof(unsigned char));
    packet_decoded = true;
  }
  return packet_decoded;
}
