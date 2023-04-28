/******************************************************************************************
 * File:        syshal_flash.cpp
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

#include "../syshal_flash.h"
#include "../syshal_config.h"

// #include <Adafruit_FlashTransport.h>
// Adafruit_FlashTransport_QSPI flashTransport;

int syshal_flash_init(void)
{
    // flashTransport.begin();
    // flashTransport.runCommand(0xB9);
    // flashTransport.end();

    return SYSHAL_FLASH_NO_ERROR;
}

int syshal_flash_term(void)
{
    return SYSHAL_FLASH_NO_ERROR;
}