/******************************************************************************************
 * File:        sysh_config_priv.h
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

#ifndef _SYS_CONFIG_PRIV_H_
#define _SYS_CONFIG_PRIV_H_

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#define SYS_CONFIG_TAG(TAG, DATA, COMPULSORY) SYS_CONFIG_TAG_GET_SET(TAG, DATA, COMPULSORY, NULL, NULL)

#define SYS_CONFIG_TAG_GET_SET(TAG, DATA, COMPULSORY, GETTER, SETTER) \
    {                                                                 \
        .tag = TAG,                                                   \
        .data = (void *)&DATA,                                        \
        .length = sizeof(DATA) - sizeof(sys_config_hdr_t),            \
        .compulsory = COMPULSORY,                                     \
        .getter = GETTER,                                             \
        .setter = SETTER                                              \
    }

#define SYS_CONFIG_REQUIRED_IF_MATCH(TAG, REQUIRED, ADDRESS, VALUE) \
    {                                                               \
        .tag = TAG,                                                 \
        .tag_dependant = REQUIRED,                                  \
        .address = (void *)&ADDRESS,                                \
        .bitmask = (__typeof__(ADDRESS))0xFFFFFFFF,                 \
        .value = VALUE                                              \
    }

#define SYS_CONFIG_REQUIRED_IF_MATCH_BITMASK(TAG, REQUIRED, ADDRESS, BITMASK, VALUE) \
    {                                                                                \
        .tag = TAG,                                                                  \
        .tag_dependant = REQUIRED,                                                   \
        .address = (void *)&ADDRESS,                                                 \
        .bitmask = (__typeof__(ADDRESS))BITMASK,                                     \
        .value = VALUE                                                               \
    }

typedef struct __attribute__((__packed__))
{
    uint16_t tag;
    void *data;
    size_t length;
    bool compulsory;
    void (*getter)(void);
    void (*setter)(void);
} sys_config_lookup_table_t;

typedef struct __attribute__((__packed__))
{
    uint16_t tag;
    uint16_t tag_dependant;
    void *address;
    uint32_t bitmask;
    uint32_t value;
} dependancy_lookup_table_t;

#define NUM_OF_TAGS (sizeof(sys_config_lookup_table) / sizeof(sys_config_lookup_table_t))
// #define NUM_OF_DEPENDENCIES (sizeof(dependancy_lookup_table) / sizeof(dependancy_lookup_table_t))

#define TAG_SET_FLAG(TAG_LOOKUP) (((sys_config_hdr_t *)TAG_LOOKUP.data)->set)

#endif /* _SYS_CONFIG_PRIV_H_ */