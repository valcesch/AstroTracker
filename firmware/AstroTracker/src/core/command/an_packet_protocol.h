/******************************************************************************************
 * File:        an_packet_protocol.h
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

#ifndef _AN_PACKET_PROTOCOL_H
#define _AN_PACKET_PROTOCOL_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#define AN_PACKET_HEADER_SIZE 5
#define AN_MAXIMUM_PACKET_SIZE 58   // Default Arduino buffer size is 64
#define AN_DECODE_BUFFER_SIZE 2 * (AN_MAXIMUM_PACKET_SIZE + AN_PACKET_HEADER_SIZE)
#define AN_DECODE_MAXIMUM_FILL_SIZE (AN_MAXIMUM_PACKET_SIZE + AN_PACKET_HEADER_SIZE)
#define an_packet_pointer(packet) (packet)->header
#define an_packet_size(packet) ((packet)->an_length + AN_PACKET_HEADER_SIZE) * sizeof(uint8_t)

#define an_decoder_pointer(an_decoder) &(an_decoder)->an_buffer[(an_decoder)->buffer_length]
#define an_decoder_size(an_decoder) (sizeof((an_decoder)->an_buffer) - (an_decoder)->buffer_length)
#define an_decoder_increment(an_decoder, bytes_received) (an_decoder)->buffer_length += bytes_received

typedef struct
{
    uint8_t an_buffer[AN_DECODE_BUFFER_SIZE];
    uint16_t buffer_length;
    uint32_t crc_errors;
} an_decoder_t;

typedef struct
{
    uint8_t id;
    uint8_t an_length;
    uint8_t header[AN_PACKET_HEADER_SIZE];
    uint8_t data[AN_MAXIMUM_PACKET_SIZE];
} an_packet_t;

void an_decoder_initialise(an_decoder_t *an_decoder);
bool an_packet_decode(an_decoder_t *an_decoder, an_packet_t *an_packet);
void an_packet_encode(an_packet_t *an_packet);

#endif
