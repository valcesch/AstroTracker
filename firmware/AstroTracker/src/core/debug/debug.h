/******************************************************************************************
 * File:        debug.h
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

#ifndef _DEBUG_H_
#define _DEBUG_H_

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "../config/sys_config.h"
#include "../../syshal/syshal_config.h"
#include "../../syshal/syshal_time.h"

#define DEBUG_NO_ERROR (0)
#define DEBUG_ERROR_DEVICE (-1)

typedef enum
{
    DEBUG_NONE,
    DEBUG_SYSTEM,
    DEBUG_ERROR,
    DEBUG_WARN,
    DEBUG_INFO,
    DEBUG_TRACE,
} debug_level_t;

extern const char * g_dbg_lvl[];
extern debug_level_t g_debug_level;

typedef struct
{
    sys_config_debug_settings_t *debug;
} syshal_debug_config_t;

int debug_init(void);
void serial_printf(const char* fmt, ...);

#ifndef DEBUG_DISABLED

#define DEBUG_PR(fmt, ...)     serial_printf(fmt "\n\r", ## __VA_ARGS__)

#ifndef DEBUG_COLOR

#define DEBUG_PR_T(lvl, fmt, ...) \
    do { \
        if (g_debug_level >= lvl) { \
        DEBUG_PR("%lu\t%s\t" fmt, \
        TIME_IN_SECONDS, \
        g_dbg_lvl[lvl], ## __VA_ARGS__); \
        } \
    } while (0)

#define DEBUG_PR_SYS(fmt, ...)       DEBUG_PR_T(DEBUG_SYSTEM, fmt, ## __VA_ARGS__)
#define DEBUG_PR_INFO(fmt, ...)      DEBUG_PR_T(DEBUG_INFO, fmt, ## __VA_ARGS__)
#define DEBUG_PR_WARN(fmt, ...)      DEBUG_PR_T(DEBUG_WARN, fmt, ## __VA_ARGS__)
#define DEBUG_PR_ERROR(fmt, ...)     DEBUG_PR_T(DEBUG_ERROR, fmt, ## __VA_ARGS__)
#define DEBUG_PR_TRACE(fmt, ...)     DEBUG_PR_T(DEBUG_TRACE, fmt, ## __VA_ARGS__)

#else

#define DEBUG_PR_T(lvl, color, fmt, ...)   if (g_debug_level >= lvl) { \
    DEBUG_PR("\e[39;49m" "%lu\t" color "%s\t" fmt, \
    TIME_IN_SECONDS, \
    g_dbg_lvl[lvl], ## __VA_ARGS__); \
}

#define DEBUG_PR_SYS(fmt, ...)       DEBUG_PR_T(DEBUG_SYSTEM, "", fmt, ## __VA_ARGS__)
#define DEBUG_PR_INFO(fmt, ...)      DEBUG_PR_T(DEBUG_INFO, "\e[38;5;4m", fmt, ## __VA_ARGS__)
#define DEBUG_PR_WARN(fmt, ...)      DEBUG_PR_T(DEBUG_WARN, "\e[38;5;130m", fmt, ## __VA_ARGS__)
#define DEBUG_PR_ERROR(fmt, ...)     DEBUG_PR_T(DEBUG_ERROR, "\e[38;5;1m", fmt, ## __VA_ARGS__)
#define DEBUG_PR_TRACE(fmt, ...)     DEBUG_PR_T(DEBUG_TRACE, "\e[38;5;8m", fmt, ## __VA_ARGS__)

#endif

#else

#define DEBUG_PR(fmt, ...)           do {} while (0)
#define DEBUG_PR_T(lvl, fmt, ...)    do {} while (0)
#define DEBUG_PR_SYS(fmt, ...)       do {} while (0)
#define DEBUG_PR_INFO(fmt, ...)      do {} while (0)
#define DEBUG_PR_WARN(fmt, ...)      do {} while (0)
#define DEBUG_PR_ERROR(fmt, ...)     do {} while (0)
#define DEBUG_PR_TRACE(fmt, ...)     do {} while (0)

#endif

#endif /* _DEBUG_H_ */

