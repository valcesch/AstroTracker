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

void COMMAND::send_msg_data_packet(msg_data_packet_t *msg_data_packet)
{
  an_packet_t an_packet;
  encode_msg_data_packet(&an_packet, msg_data_packet);
  an_packet_transmit(&an_packet);
}

uint8_t COMMAND::receive_msg_data_packet(msg_data_packet_t *msg_data_packet)
{
  an_packet_t an_packet;
  if (an_packet_receive(&an_packet, sizeof(msg_data_packet_t)) &&
      decode_msg_data_packet(msg_data_packet, &an_packet))
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 0);
    return CMD_NO_ERROR;
  }
  else
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 1);
    return CMD_ERROR_AN_PACKET_NOT_VALID;
  }
}

void COMMAND::send_config_packet(config_packet_t *config_packet)
{
  an_packet_t an_packet;
  encode_config_packet(&an_packet, config_packet);
  an_packet_transmit(&an_packet);
}

uint8_t COMMAND::receive_config_packet(config_packet_t *config_packet)
{
  an_packet_t an_packet;
  if (an_packet_receive(&an_packet, sizeof(config_packet_t)) &&
      decode_config_packet(config_packet, &an_packet))
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 0);
    return CMD_NO_ERROR;
  }
  else
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 1);
    return CMD_ERROR_AN_PACKET_NOT_VALID;
  }
}

void COMMAND::send_cmd_data_packet(cmd_data_packet_t *cmd_data_packet)
{
  an_packet_t an_packet;
  encode_cmd_data_packet(&an_packet, cmd_data_packet);
  an_packet_transmit(&an_packet);
}

uint8_t COMMAND::receive_cmd_data_packet(cmd_data_packet_t *cmd_data_packet)
{
  an_packet_t an_packet;
  if (an_packet_receive(&an_packet, sizeof(cmd_data_packet_t)) &&
      decode_cmd_data_packet(cmd_data_packet, &an_packet))
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 0);
    return CMD_NO_ERROR;
  }
  else
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 1);
    return CMD_ERROR_AN_PACKET_NOT_VALID;
  }
}

void COMMAND::send_sat_status_packet(sat_status_packet_t *sat_status_packet)
{
  an_packet_t an_packet;
  encode_sat_status_packet(&an_packet, sat_status_packet);
  an_packet_transmit(&an_packet);
}

uint8_t COMMAND::receive_sat_status_packet(sat_status_packet_t *sat_status_packet)
{
  an_packet_t an_packet;
  if (an_packet_receive(&an_packet, sizeof(sat_status_packet_t)) &&
      decode_sat_status_packet(sat_status_packet, &an_packet))
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 0);
    return CMD_NO_ERROR;
  }
  else
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 1);
    return CMD_ERROR_AN_PACKET_NOT_VALID;
  }
}

void COMMAND::send_logger_status_packet(logger_status_packet_t *logger_status_packet)
{
  an_packet_t an_packet;
  encode_logger_status_packet(&an_packet, logger_status_packet);
  an_packet_transmit(&an_packet);
}

uint8_t COMMAND::receive_logger_status_packet(logger_status_packet_t *logger_status_packet)
{
  an_packet_t an_packet;
  if (an_packet_receive(&an_packet, sizeof(logger_status_packet_t)) &&
      decode_logger_status_packet(logger_status_packet, &an_packet))
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 0);
    return CMD_NO_ERROR;
  }
  else
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 1);
    return CMD_ERROR_AN_PACKET_NOT_VALID;
  }
}

void COMMAND::send_gps_status_packet(gps_status_packet_t *gps_status_packet)
{
  an_packet_t an_packet;
  encode_gps_status_packet(&an_packet, gps_status_packet);
  an_packet_transmit(&an_packet);
}

uint8_t COMMAND::receive_gps_status_packet(gps_status_packet_t *gps_status_packet)
{
  an_packet_t an_packet;
  if (an_packet_receive(&an_packet, sizeof(gps_status_packet_t)) &&
      decode_gps_status_packet(gps_status_packet, &an_packet))
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 0);
    return CMD_NO_ERROR;
  }
  else
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 1);
    return CMD_ERROR_AN_PACKET_NOT_VALID;
  }
}

void COMMAND::send_ble_status_packet(ble_status_packet_t *ble_status_packet)
{
  an_packet_t an_packet;
  encode_ble_status_packet(&an_packet, ble_status_packet);
  an_packet_transmit(&an_packet);
}

uint8_t COMMAND::receive_ble_status_packet(ble_status_packet_t *ble_status_packet)
{
  an_packet_t an_packet;
  if (an_packet_receive(&an_packet, sizeof(ble_status_packet_t)) &&
      decode_ble_status_packet(ble_status_packet, &an_packet))
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 0);
    return CMD_NO_ERROR;
  }
  else
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 1);
    return CMD_ERROR_AN_PACKET_NOT_VALID;
  }
}

void COMMAND::send_asset_status_packet(asset_status_packet_t *asset_status_packet)
{
  an_packet_t an_packet;
  encode_asset_status_packet(&an_packet, asset_status_packet);
  an_packet_transmit(&an_packet);
}

uint8_t COMMAND::receive_asset_status_packet(asset_status_packet_t *asset_status_packet)
{
  an_packet_t an_packet;
  if (an_packet_receive(&an_packet, sizeof(asset_status_packet_t)) &&
      decode_asset_status_packet(asset_status_packet, &an_packet))
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 0);
    return CMD_NO_ERROR;
  }
  else
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 1);
    return CMD_ERROR_AN_PACKET_NOT_VALID;
  }
}

void COMMAND::send_sat_bulletin_packet(sat_bulletin_packet_t *sat_bulletin_packet)
{
  an_packet_t an_packet;
  encode_sat_bulletin_packet(&an_packet, sat_bulletin_packet);
  an_packet_transmit(&an_packet);
}

uint8_t COMMAND::receive_sat_bulletin_packet(sat_bulletin_packet_t *sat_bulletin_packet)
{
  an_packet_t an_packet;
  if (an_packet_receive(&an_packet, sizeof(sat_bulletin_packet_t)) &&
      decode_sat_bulletin_packet(sat_bulletin_packet, &an_packet))
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 0);
    return CMD_NO_ERROR;
  }
  else
  {
    if (_with_cmd_ack == true)
      send_packet_acknowledge(an_packet, 1);
    return CMD_ERROR_AN_PACKET_NOT_VALID;
  }
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