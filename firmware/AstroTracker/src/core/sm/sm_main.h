/******************************************************************************************
 * File:        sm_main.h
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

#ifndef _SM_MAIN_H_
#define _SM_MAIN_H_

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "sm.h"
#include "../cexception/exceptions.h"
#include "../cexception/cexception.h"

extern sm_state_func_t sm_main_states[]; // State function lookup table is populate in sm_main.c

typedef enum
{
    SM_MAIN_BOOT,
    SM_MAIN_ERROR,
    SM_MAIN_BATTERY_LEVEL_LOW,
    SM_MAIN_LOG_FILE_FULL,
    SM_MAIN_PROVISIONING,
    SM_MAIN_OPERATIONAL,
    SM_MAIN_DEACTIVATE,
} sm_main_states_t;

void sm_main_exception_handler(CEXCEPTION_T e);

#endif /* _SM_MAIN_H_ */
