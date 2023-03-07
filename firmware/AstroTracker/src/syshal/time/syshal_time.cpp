/******************************************************************************************
 * File:        syshal_time.cpp
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

#include "../syshal_time.h"
#include "../../core/debug/debug.h"
#include "../syshal_config.h"

static bool is_init = false;

int syshal_time_init(void)
{
    // Empty

    is_init = true;

    return SYSHAL_TIME_NO_ERROR;
}

int syshal_time_term(void)
{
    if (is_init)
    {
        // Empty
    }

    is_init = false;

    return SYSHAL_TIME_NO_ERROR;
}

uint32_t syshal_time_get_ticks_ms(void)
{
    return millis();
}

uint32_t syshal_time_get_ticks_us(void)
{
    return micros();
}

void syshal_time_delay_us(uint32_t us)
{
    delayMicroseconds(us);
}

void syshal_time_delay_ms(uint32_t ms)
{
    delay(ms);
}
