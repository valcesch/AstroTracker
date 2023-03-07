/******************************************************************************************
 * File:        syshal_batt.cpp
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

#include "../syshal_batt.h"
#include "../syshal_gpio.h"
#include "../../core/debug/debug.h"
#include "../syshal_config.h"

#define SYSHAL_BATT_GPIO_ADC_BATT (GPIO_VBAT)

// Private functions
// ...

int syshal_batt_init(void)
{
    syshal_gpio_init(SYSHAL_BATT_GPIO_ADC_BATT, INPUT);

    return SYSHAL_BATT_NO_ERROR;
}

int syshal_batt_term(void)
{
    return SYSHAL_BATT_NO_ERROR;
}

int syshal_batt_voltage(uint8_t *voltage)
{
    float measuredvbat = (float)(syshal_gpio_analog_read(SYSHAL_BATT_GPIO_ADC_BATT));
    measuredvbat *= 2;    // we divided by 2, so multiply back
    measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
    measuredvbat /= 1024; // convert to voltage
    *voltage = (uint8_t)(measuredvbat * 10.0);

    // DEBUG_PR_TRACE("Read battery voltage: %d", *voltage);

    return SYSHAL_BATT_NO_ERROR;
}

int syshal_batt_level(uint8_t *level)
{
    *level = 0; // Not implemented

    // DEBUG_PR_TRACE("Read battery level: %d", *level);

    return SYSHAL_BATT_NO_ERROR;
}