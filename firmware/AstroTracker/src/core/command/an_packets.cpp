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
  memcpy(an_packet->data, cmd_data_packet, sizeof(cmd_data_packet_t));
}

uint8_t decode_cmd_data_packet(cmd_data_packet_t *cmd_data_packet, an_packet_t *an_packet)
{
  uint8_t packet_decoded = false;
  if (an_packet->id == packet_id_cmd_data && an_packet->an_length == sizeof(cmd_data_packet_t))
  {
    memcpy(cmd_data_packet, an_packet->data, sizeof(cmd_data_packet_t));
    packet_decoded = true;
  }
  return packet_decoded;
}

void encode_msg_data_packet(an_packet_t *an_packet, msg_data_packet_t *msg_data_packet)
{
  an_packet->id = packet_id_msg_data;
  an_packet->an_length = sizeof(msg_data_packet_t);
  memcpy(an_packet->data, msg_data_packet, sizeof(msg_data_packet_t));
}

uint8_t decode_msg_data_packet(msg_data_packet_t *msg_data_packet, an_packet_t *an_packet)
{
  uint8_t packet_decoded = false;
  if (an_packet->id == packet_id_msg_data && an_packet->an_length == sizeof(msg_data_packet_t))
  {
    memcpy(msg_data_packet, an_packet->data, sizeof(msg_data_packet_t));
    packet_decoded = true;
  }
  return packet_decoded;
}

void encode_sat_status_packet(an_packet_t *an_packet, sat_status_packet_t *sat_status_packet)
{
  an_packet->id = packet_id_sat_status;
  an_packet->an_length = sizeof(sat_status_packet_t);
  memcpy(an_packet->data, sat_status_packet, sizeof(sat_status_packet_t));
}

uint8_t decode_sat_status_packet(sat_status_packet_t *sat_status_packet, an_packet_t *an_packet)
{
  uint8_t packet_decoded = false;
  if (an_packet->id == packet_id_sat_status && an_packet->an_length == sizeof(sat_status_packet_t))
  {
    memcpy(sat_status_packet, an_packet->data, sizeof(sat_status_packet_t));
    packet_decoded = true;
  }
  return packet_decoded;
}

void encode_logger_status_packet(an_packet_t *an_packet, logger_status_packet_t *logger_status_packet)
{
  an_packet->id = packet_id_logger_status;
  an_packet->an_length = sizeof(logger_status_packet_t);
  memcpy(an_packet->data, logger_status_packet, sizeof(logger_status_packet_t));
}

uint8_t decode_logger_status_packet(logger_status_packet_t *logger_status_packet, an_packet_t *an_packet)
{
  uint8_t packet_decoded = false;
  if (an_packet->id == packet_id_logger_status && an_packet->an_length == sizeof(logger_status_packet_t))
  {
    memcpy(logger_status_packet, an_packet->data, sizeof(logger_status_packet_t));
    packet_decoded = true;
  }
  return packet_decoded;
}

void encode_gps_status_packet(an_packet_t *an_packet, gps_status_packet_t *gps_status_packet)
{
  an_packet->id = packet_id_gps_status;
  an_packet->an_length = sizeof(gps_status_packet_t);
  memcpy(an_packet->data, gps_status_packet, sizeof(gps_status_packet_t));
}

uint8_t decode_gps_status_packet(gps_status_packet_t *gps_status_packet, an_packet_t *an_packet)
{
  uint8_t packet_decoded = false;
  if (an_packet->id == packet_id_gps_status && an_packet->an_length == sizeof(gps_status_packet_t))
  {
    memcpy(gps_status_packet, an_packet->data, sizeof(gps_status_packet_t));
    packet_decoded = true;
  }
  return packet_decoded;
}

void encode_ble_status_packet(an_packet_t *an_packet, ble_status_packet_t *ble_status_packet)
{
  an_packet->id = packet_id_ble_status;
  an_packet->an_length = sizeof(ble_status_packet_t);
  memcpy(an_packet->data, ble_status_packet, sizeof(ble_status_packet_t));
}

uint8_t decode_ble_status_packet(ble_status_packet_t *ble_status_packet, an_packet_t *an_packet)
{
  uint8_t packet_decoded = false;
  if (an_packet->id == packet_id_ble_status && an_packet->an_length == sizeof(ble_status_packet_t))
  {
    memcpy(ble_status_packet, an_packet->data, sizeof(ble_status_packet_t));
    packet_decoded = true;
  }
  return packet_decoded;
}

void encode_asset_status_packet(an_packet_t *an_packet, asset_status_packet_t *asset_status_packet)
{
  an_packet->id = packet_id_asset_status;
  an_packet->an_length = sizeof(asset_status_packet_t);
  memcpy(an_packet->data, asset_status_packet, sizeof(asset_status_packet_t));
}

uint8_t decode_asset_status_packet(asset_status_packet_t *asset_status_packet, an_packet_t *an_packet)
{
  uint8_t packet_decoded = false;
  if (an_packet->id == packet_id_asset_status && an_packet->an_length == sizeof(asset_status_packet_t))
  {
    memcpy(asset_status_packet, an_packet->data, sizeof(asset_status_packet_t));
    packet_decoded = true;
  }
  return packet_decoded;
}

void encode_sat_bulletin_packet(an_packet_t *an_packet, sat_bulletin_packet_t *sat_bulletin_packet)
{
  an_packet->id = packet_id_sat_bulletin;
  an_packet->an_length = sizeof(sat_bulletin_packet_t);
  memcpy(an_packet->data, sat_bulletin_packet, sizeof(sat_bulletin_packet_t));
}

uint8_t decode_sat_bulletin_packet(sat_bulletin_packet_t *sat_bulletin_packet, an_packet_t *an_packet)
{
  uint8_t packet_decoded = false;
  if (an_packet->id == packet_id_sat_bulletin && an_packet->an_length == sizeof(sat_bulletin_packet_t))
  {
    memcpy(sat_bulletin_packet, an_packet->data, sizeof(sat_bulletin_packet_t));
    packet_decoded = true;
  }
  return packet_decoded;
}

void encode_config_packet(an_packet_t *an_packet, config_packet_t *config_packet)
{
  an_packet->id = packet_id_config;
  an_packet->an_length = sizeof(config_packet_t);
  memcpy(an_packet->data, config_packet, sizeof(config_packet_t));
}

uint8_t decode_config_packet(config_packet_t *config_packet, an_packet_t *an_packet)
{
  uint8_t packet_decoded = false;
  if (an_packet->id == packet_id_config && an_packet->an_length == sizeof(config_packet_t))
  {
    memcpy(config_packet, an_packet->data, sizeof(config_packet_t));
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
