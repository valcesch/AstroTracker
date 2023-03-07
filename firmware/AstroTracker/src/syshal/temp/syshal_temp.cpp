/******************************************************************************************
 * File:        syshal_temp.cpp
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

#include "../syshal_temp.h"
#include "../syshal_config.h"
#include "../../core/debug/debug.h"
#if defined(NRF52_SERIES)
#include <nrf_temp.h>
#elif defined(ARDUINO_ARCH_SAMD)
#include "TemperatureZero.h"
TemperatureZero TempZero = TemperatureZero();
#endif

int syshal_temp_init(void)
{
#if defined(NRF52_SERIES)
    // Using ARM built in functions
#elif defined(ARDUINO_ARCH_SAMD)
    TempZero.init();
    TempZero.disable();
#endif

    return SYSHAL_TEMP_NO_ERROR;
}

int syshal_temp_temperature(int8_t *temperature)
{
    float temp = 0;

#if defined(NRF52_SERIES)
    int32_t result;
    /*
    NRF_TEMP->TASKS_START = 1; // Start temperature measurement
    while (NRF_TEMP->EVENTS_DATARDY == 0)
    {
    }
    NRF_TEMP->EVENTS_DATARDY = 0; // Temperature measurement complete, data ready
    result = NRF_TEMP->TEMP;
    NRF_TEMP->TASKS_STOP = 1; // Stop temperature measurement
    */
#ifdef _VARIANT_FEATHER52840_
    sd_temp_get(&result);
#endif

    temp = result * 0.25;
#elif defined(ARDUINO_ARCH_SAMD)
    TempZero.wakeup();
    temp = TempZero.readInternalTemperature();
    TempZero.disable();
#endif

    *temperature = (int8_t)temp;

    DEBUG_PR_TRACE("Read temperature %d", *temperature);

    return SYSHAL_TEMP_NO_ERROR;
}