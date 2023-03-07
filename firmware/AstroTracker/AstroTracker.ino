/******************************************************************************************
 * File:        main.cpp
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

#include "src/core/sm/sm.h"
#include "src/core/sm/sm_main.h"
#include "src/core/cexception/cexception.h"
#include "src/syshal/syshal_pmu.h"

void setup()
{
  // Empty
}

void loop()
{
  sm_handle_t state_handle;

  sm_init(&state_handle, sm_main_states);
  sm_set_next_state(&state_handle, SM_MAIN_BOOT);

  while (1)
  {
    CEXCEPTION_T e = CEXCEPTION_NONE;

    Try
    {
      sm_tick(&state_handle);
    }
    Catch(e)
    {
      sm_main_exception_handler(e);
    }
  }
}
