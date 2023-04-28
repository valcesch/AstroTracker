/******************************************************************************************
 * File:        sys_config.cpp
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

#include <string.h>
#include "sys_config.h"
#include "sys_config_priv.h"
#include "../debug/debug.h"

// Exposed variables
sys_config_t sys_config; // Configuration data stored in RAM

static const sys_config_lookup_table_t sys_config_lookup_table[] =
    {
        SYS_CONFIG_TAG(SYS_CONFIG_TAG_DEBUG_SETTINGS, sys_config.debug_settings, false),
        SYS_CONFIG_TAG(SYS_CONFIG_TAG_BLE_SETTINGS, sys_config.ble_settings, true),
        SYS_CONFIG_TAG(SYS_CONFIG_TAG_GPS_SETTINGS, sys_config.gps_settings, true),
        SYS_CONFIG_TAG(SYS_CONFIG_TAG_SAT_SETTINGS, sys_config.sat_settings, true),
        SYS_CONFIG_TAG(SYS_CONFIG_TAG_SCREEN_SETTINGS, sys_config.screen_settings, true),
        SYS_CONFIG_TAG(SYS_CONFIG_TAG_SATPASS_SETTINGS, sys_config.satpass_settings, true),
        SYS_CONFIG_TAG(SYS_CONFIG_TAG_SCHEDULER_GPS_SETTINGS, sys_config.gps_scheduler_settings, true),
        SYS_CONFIG_TAG(SYS_CONFIG_TAG_SCHEDULER_SATPASS_SETTINGS, sys_config.satpass_scheduler_settings, false),
        SYS_CONFIG_TAG(SYS_CONFIG_TAG_BATTERY_LOW_THRESHOLD, sys_config.battery_low_threshold, false),
        SYS_CONFIG_TAG(SYS_CONFIG_TAG_GPS_LOG_POSITION_ENABLE, sys_config.gps_log_position_enable, false),
        SYS_CONFIG_TAG(SYS_CONFIG_TAG_SATPASS_PREDICTOR_ENABLE, sys_config.satpass_predictor_enable, false),
        SYS_CONFIG_TAG(SYS_CONFIG_TAG_LOGGING_ENABLE, sys_config.logging_enable, false),
};

static int sys_config_get_index(uint16_t tag, uint32_t *index)
{
    for (uint32_t i = 0; i < NUM_OF_TAGS; ++i)
    {
        if (sys_config_lookup_table[i].tag == tag)
        {
            *index = i;
            return SYS_CONFIG_NO_ERROR;
        }
    }

    return SYS_CONFIG_ERROR_INVALID_TAG;
}

bool sys_config_exists(uint16_t tag)
{
    uint32_t index;
    int ret = sys_config_get_index(tag, &index);

    if (SYS_CONFIG_NO_ERROR == ret)
        return true;
    else
        return false;
}

int sys_config_is_set(uint16_t tag, bool *set)
{
    uint32_t index;
    int ret = sys_config_get_index(tag, &index);

    if (ret)
        return ret;

    *set = TAG_SET_FLAG(sys_config_lookup_table[index]);
    return SYS_CONFIG_NO_ERROR;
}

int sys_config_unset(uint16_t tag)
{
    uint32_t index;
    int ret = sys_config_get_index(tag, &index);

    if (ret)
        return ret;

    TAG_SET_FLAG(sys_config_lookup_table[index]) = false;
    return SYS_CONFIG_NO_ERROR;
}

int sys_config_size(uint16_t tag, size_t *size)
{
    uint32_t index;
    int ret = sys_config_get_index(tag, &index);

    if (ret)
        return ret;

    *size = sys_config_lookup_table[index].length;
    return SYS_CONFIG_NO_ERROR;
}

int sys_config_is_required(uint16_t tag, bool *required)
{
    uint32_t index;
    int ret = sys_config_get_index(tag, &index);

    // Is this tag valid?
    if (ret)
        return ret;

    // Is this tag compulsory?
    if (sys_config_lookup_table[index].compulsory && !TAG_SET_FLAG(sys_config_lookup_table[index]))
    {
        // If it's not set then it is required
        *required = true;
        return SYS_CONFIG_NO_ERROR;
    }

    *required = false;

    /*
    // Scan through all the dependencies to see if another tag requires this one
    for (uint32_t i = 0; i < NUM_OF_DEPENDENCIES; ++i)
    {
        // If there's a mention of this tag in a dependency
        if (dependancy_lookup_table[i].tag == tag)
        {
            // Is this other tag valid?
            bool is_set;
            if (SYS_CONFIG_NO_ERROR == sys_config_is_set(dependancy_lookup_table[i].tag_dependant, &is_set))
            {
                // Is this other tag set?
                if (!is_set)
                    continue;

                // Does this tag value match that which is required to trigger this dependancy?
                if ((*(uint32_t *)dependancy_lookup_table[i].address & dependancy_lookup_table[i].bitmask) == dependancy_lookup_table[i].value)
                {
                    // Is the tag it relies on set and thus it's dependency sorted?
                    if (sys_config_is_set(dependancy_lookup_table[i].tag, &is_set))
                        continue;

                    if (is_set)
                        continue;

                    // A dependency is not fulfilled
                    *required = true;
                    break;
                }
            }
        }
    }
    */

    return SYS_CONFIG_NO_ERROR;
}

int sys_config_get(uint16_t tag, void **value)
{
    uint32_t index;
    int ret = sys_config_get_index(tag, &index);

    if (ret)
        return ret;

    if (sys_config_lookup_table[index].getter)
        sys_config_lookup_table[index].getter();

    if (!TAG_SET_FLAG(sys_config_lookup_table[index]))
        return SYS_CONFIG_ERROR_TAG_NOT_SET;

    // Get the address of the configuration data in RAM
    if (value != NULL)
        (*value) = ((uint8_t *)sys_config_lookup_table[index].data) + sizeof(sys_config_hdr_t);

    return sys_config_lookup_table[index].length;
}

int sys_config_set(uint16_t tag, const void *data, size_t length)
{
    uint32_t index;
    int ret = sys_config_get_index(tag, &index);

    if (ret)
        return ret;

    if (sys_config_lookup_table[index].length != length)
        return SYS_CONFIG_ERROR_WRONG_SIZE;

    // Copy in the data to the contents section
    memcpy(((uint8_t *)sys_config_lookup_table[index].data) + sizeof(sys_config_hdr_t), data, length);

    // Set the set flag
    TAG_SET_FLAG(sys_config_lookup_table[index]) = true;

    if (sys_config_lookup_table[index].setter)
        sys_config_lookup_table[index].setter();

    return SYS_CONFIG_NO_ERROR;
}

int sys_config_iterate(uint16_t *tag, uint16_t *last_index)
{
    uint16_t idx = *last_index;
    if (idx >= NUM_OF_TAGS)
        return SYS_CONFIG_ERROR_NO_MORE_TAGS;

    // Return the next tag in the list
    (*last_index)++;
    *tag = sys_config_lookup_table[idx].tag;

    return SYS_CONFIG_NO_ERROR;
}