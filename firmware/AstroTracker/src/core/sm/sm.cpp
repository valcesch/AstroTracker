/* sm.c - State machine handling code
 *
 * Copyright (C) 2018 Arribada
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "sm.h"
#include <limits.h>

#define FUNCTION_NOT_SET (INT_MAX)

void sm_init(sm_handle_t *handle, sm_state_func_t *state_table)
{
    handle->state_function_lookup_table = state_table;
    handle->last_state = FUNCTION_NOT_SET;
    handle->current_state = FUNCTION_NOT_SET;
    handle->next_state = FUNCTION_NOT_SET;
    handle->first_time_running = true;
}

void sm_tick(sm_handle_t *handle)
{
    handle->state_function_lookup_table[handle->current_state](handle);

    handle->first_time_running = false;

    if (handle->current_state != handle->next_state)
    {
        handle->first_time_running = true;
        handle->last_state = handle->current_state;
        handle->current_state = handle->next_state;
    }
};

bool sm_is_first_entry(const sm_handle_t *handle)
{
    return handle->first_time_running;
}

bool sm_is_last_entry(const sm_handle_t *handle)
{
    return sm_get_next_state(handle) != sm_get_current_state(handle);
}

void sm_set_next_state(sm_handle_t *handle, int state)
{
    if (FUNCTION_NOT_SET == handle->current_state)
        handle->current_state = state;

    handle->next_state = state;
}

int sm_get_last_state(const sm_handle_t *handle)
{
    return handle->last_state;
}

int sm_get_current_state(const sm_handle_t *handle)
{
    return handle->current_state;
}

int sm_get_next_state(const sm_handle_t *handle)
{
    return handle->next_state;
}