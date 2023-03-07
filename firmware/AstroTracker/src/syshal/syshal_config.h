/******************************************************************************************
 * File:        syshal_config.h
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

#ifndef _SYSHAL_CONFIG_H_
#define _SYSHAL_CONFIG_H_

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

// Constants
// AstroTracker definitions
#define GPIO_GPS_EXT_INT            (1u)
#define GPIO_GPS_EN                 (5u)
#define GPIO_LED                    (LED_BUILTIN)
#define GPIO_ANS_EXT_INT            (8u)
#define GPIO_ANS_ANT_IN_USE         (9u)
#define GPIO_ANS_EN                 (4u)
#define GPIO_ANS_RESET              (3u)
#define GPIO_HWDT_RESET             (0u)
#define GPIO_VBAT                   (A4)
#define GPIO_ANS_UART_RX            (PIN_SERIAL_RX)
#define GPIO_ANS_UART_TX            (PIN_SERIAL_TX)

#define UART_DEBUG                  Serial
#define UART_ANS                    Serial5
#define I2C_GNSS                    Wire

#endif