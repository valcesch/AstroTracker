/******************************************************************************************
 * File:        an_packet_protocol.cpp
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

#include "an_packet_protocol.h"

/*
   Function to calculate crc16 over provided table
*/
uint16_t calculate_crc16(uint8_t *data, uint16_t an_length)
{
  uint16_t x;
  uint16_t crc = 0xFFFF;

  while (an_length--)
  {
    x = crc >> 8 ^ *data++;
    x ^= x >> 4;
    crc = (crc << 8) ^ (x << 12) ^ (x << 5) ^ (x);
  }

  return crc;
}

/*
   Function to calculate a 4 byte LRC
*/
uint8_t calculate_header_lrc(uint8_t *data)
{
  return ((data[0] + data[1] + data[2] + data[3]) ^ 0xFF) + 1;
}

/*
   Initialise the decoder
*/
void an_decoder_initialise(an_decoder_t *an_decoder)
{
  an_decoder->buffer_length = 0;
  an_decoder->crc_errors = 0;
}

/*
   Function to decode an_packets from raw data
   To fill buffer set an_packet to NULL
   To decode packets set buffer to NULL and buffer_length to zero
   returns TRUE (1) if a packet was decoded or FALSE (0) if no packet was decoded
*/
bool an_packet_decode(an_decoder_t *an_decoder, an_packet_t *an_packet)
{
  uint16_t decode_iterator = 0;
  bool packet_decoded = false;
  uint8_t header_lrc;
  uint16_t crc;

  while (decode_iterator + AN_PACKET_HEADER_SIZE <= an_decoder->buffer_length)
  {
    header_lrc = an_decoder->an_buffer[decode_iterator++];
    if (header_lrc == calculate_header_lrc(&an_decoder->an_buffer[decode_iterator]))
    {
      an_packet->id = an_decoder->an_buffer[decode_iterator++];
      an_packet->an_length = an_decoder->an_buffer[decode_iterator++];
      crc = an_decoder->an_buffer[decode_iterator++];
      crc |= an_decoder->an_buffer[decode_iterator++] << 8;

      if (decode_iterator + an_packet->an_length > an_decoder->buffer_length)
      {
        decode_iterator -= AN_PACKET_HEADER_SIZE;
        break;
      }

      if (crc == calculate_crc16(&an_decoder->an_buffer[decode_iterator], an_packet->an_length))
      {
        packet_decoded = true;
        memcpy(an_packet->header, &an_decoder->an_buffer[decode_iterator - AN_PACKET_HEADER_SIZE], AN_PACKET_HEADER_SIZE * sizeof(uint8_t));
        memcpy(an_packet->data, &an_decoder->an_buffer[decode_iterator], an_packet->an_length * sizeof(uint8_t));
        decode_iterator += an_packet->an_length;
        break;
      }
      else
      {
        decode_iterator -= (AN_PACKET_HEADER_SIZE - 1);
        an_decoder->crc_errors++;
      }
    }
  }
  if (decode_iterator < an_decoder->buffer_length)
  {
    if (decode_iterator > 0)
    {
      memmove(&an_decoder->an_buffer[0], &an_decoder->an_buffer[decode_iterator], (an_decoder->buffer_length - decode_iterator) * sizeof(uint8_t));
      an_decoder->buffer_length -= decode_iterator;
    }
  }
  else
    an_decoder->buffer_length = 0;

  return packet_decoded;
}

/*
   Function to encode an an_packet
*/
void an_packet_encode(an_packet_t *an_packet)
{
  uint16_t crc;
  an_packet->header[1] = an_packet->id;
  an_packet->header[2] = an_packet->an_length;
  crc = calculate_crc16(an_packet->data, an_packet->an_length);
  memcpy(&an_packet->header[3], &crc, sizeof(uint16_t));
  an_packet->header[0] = calculate_header_lrc(&an_packet->header[1]);
}
