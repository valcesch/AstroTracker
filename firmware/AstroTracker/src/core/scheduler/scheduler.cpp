/******************************************************************************************
 * File:        scheduler.h
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

#include "scheduler.h"
#include "CronAlarms.h"
#include "../debug/debug.h"
#include "../../syshal/syshal_rtc.h"

CronClass Cron = CronClass(syshal_rtc_return_timestamp);

static scheduler_gps_config_t gps_config;
static scheduler_satpass_config_t satpass_config;

CronId scheduler_alarm_gps_start_id = dtINVALID_ALARM_ID;
CronId scheduler_alarm_satpass_start_id = dtINVALID_ALARM_ID;

char scheduler_alarm_gps_start_cron_expr[18];
char scheduler_alarm_satpass_start_cron_expr[18];

#define SCHEDULER_ALARM_TIMEOUT_S 900

// Private functions
int scheduler_gps_set_alarm_config_priv(uint8_t interval_h);
void scheduler_gps_start_callback_priv(void);
int scheduler_satpass_set_alarm_config_priv(uint32_t timestamp);
void scheduler_satpass_start_callback_priv(void);

int scheduler_init(void)
{
    // Empty

    return SCHEDULER_NO_ERROR;
}

int scheduler_gps_update_config(scheduler_gps_config_t scheduler_gps_config)
{
    DEBUG_PR_TRACE("Update GPS configuration. %s()", __FUNCTION__);

    gps_config = scheduler_gps_config;

    if (gps_config.scheduler->hdr.set)
    {
        DEBUG_PR_TRACE("Update GPS alarm. %s()", __FUNCTION__);
        if (!scheduler_gps_set_alarm_config_priv(gps_config.scheduler->contents.interval_h))
        {
            // Restart GPS scheduler if needed
            if (Cron.isAllocated(scheduler_alarm_gps_start_id) == true)
            {
                scheduler_gps_stop();
                if (scheduler_gps_start())
                    return SCHEDULER_ERROR_INVALID_PARAM;
            }
        }
    }

    return SCHEDULER_NO_ERROR;
}

int scheduler_satpass_update_config(scheduler_satpass_config_t scheduler_satpass_config)
{
    DEBUG_PR_TRACE("Update SATPASS configuration. %s()", __FUNCTION__);

    satpass_config = scheduler_satpass_config;

    if (satpass_config.scheduler->hdr.set)
    {
        DEBUG_PR_TRACE("Update SATPASS alarm. %s()", __FUNCTION__);
        if (!scheduler_satpass_set_alarm_config_priv(satpass_config.scheduler->contents.timestamp))
        {
            // Restart SATPASS scheduler if needed
            if ((Cron.isAllocated(scheduler_alarm_satpass_start_id) == true))
            {
                scheduler_satpass_stop();
                if (scheduler_satpass_start())
                    return SCHEDULER_ERROR_INVALID_PARAM;
            }
        }
    }

    return SCHEDULER_NO_ERROR;
}

int scheduler_term(void)
{
    DEBUG_PR_WARN("%s Not implemented", __FUNCTION__);

    return SCHEDULER_NO_ERROR;
}

int scheduler_get_timestamp_next_alarm(uint32_t *timestamp)
{
    CronId alarm_next_trigger_id;
    *timestamp = (uint32_t)Cron.getNextTrigger(&alarm_next_trigger_id);
    return SCHEDULER_NO_ERROR;
}

int scheduler_gps_start(void)
{
    DEBUG_PR_TRACE("%s() called.", __FUNCTION__);

    // Schedule scheduler GPS start
    if (Cron.isAllocated(scheduler_alarm_gps_start_id) == false)
    {
        scheduler_alarm_gps_start_id = Cron.create(scheduler_alarm_gps_start_cron_expr,
                                                   scheduler_gps_start_callback_priv,
                                                   false);

        if (scheduler_alarm_gps_start_id == dtINVALID_ALARM_ID)
        {
            DEBUG_PR_WARN("Job cannot be scheduled. %s", __FUNCTION__);
            return SCHEDULER_ERROR_INVALID_STATE;
        }
    }

    return SCHEDULER_NO_ERROR;
}

int scheduler_gps_stop(void)
{
    DEBUG_PR_TRACE("%s() called.", __FUNCTION__);

    if (Cron.isAllocated(scheduler_alarm_gps_start_id))
    {
        Cron.free(scheduler_alarm_gps_start_id);
        scheduler_alarm_gps_start_id = dtINVALID_ALARM_ID;
    }

    return SCHEDULER_NO_ERROR;
}

int scheduler_satpass_start(void)
{
    DEBUG_PR_TRACE("%s() called.", __FUNCTION__);

    // Schedule scheduler SATPASS start
    if (Cron.isAllocated(scheduler_alarm_satpass_start_id) == false)
    {
        scheduler_alarm_satpass_start_id = Cron.create(scheduler_alarm_satpass_start_cron_expr,
                                                       scheduler_satpass_start_callback_priv,
                                                       true);

        if (scheduler_alarm_satpass_start_id == dtINVALID_ALARM_ID)
        {
            DEBUG_PR_WARN("Job cannot be scheduled. %s", __FUNCTION__);
            return SCHEDULER_ERROR_INVALID_STATE;
        }
    }

    return SCHEDULER_NO_ERROR;
}

int scheduler_satpass_stop(void)
{
    DEBUG_PR_TRACE("%s() called.", __FUNCTION__);

    if (Cron.isAllocated(scheduler_alarm_satpass_start_id))
    {
        Cron.free(scheduler_alarm_satpass_start_id);
        scheduler_alarm_satpass_start_id = dtINVALID_ALARM_ID;
    }

    return SCHEDULER_NO_ERROR;
}

int scheduler_gps_set_alarm_config_priv(uint8_t interval_h)
{
    DEBUG_PR_TRACE("Setting new rates for GPS. %s", __FUNCTION__);

    if ((interval_h >= 1) && (interval_h <= 24))
    {
        sprintf(scheduler_alarm_gps_start_cron_expr, "0 0 */%d * * *", interval_h);
        DEBUG_PR_TRACE("Scheduler alarm: %s.", scheduler_alarm_gps_start_cron_expr);
    }
    else
    {
        DEBUG_PR_ERROR("Alarm not valid. Ignored.");
        return SCHEDULER_ERROR_INVALID_PARAM;
    }

    return SCHEDULER_NO_ERROR;
}

int scheduler_satpass_set_alarm_config_priv(uint32_t timestamp)
{
    DEBUG_PR_TRACE("Setting new ALARM for SATPASS. %s", __FUNCTION__);

    if ((timestamp > 0) && (timestamp > syshal_rtc_return_timestamp()))
    {
        time_t timestamp_t = timestamp;
        sprintf(scheduler_alarm_satpass_start_cron_expr, "%2d %2d %2d %2d %2d *", second(timestamp_t), minute(timestamp_t), hour(timestamp_t), day(timestamp_t), month(timestamp_t));
        DEBUG_PR_TRACE("Scheduler alarm: %s.", scheduler_alarm_satpass_start_cron_expr);
    }
    else
    {
        DEBUG_PR_ERROR("Alarm not valid. Ignored.");
        return SCHEDULER_ERROR_INVALID_PARAM;
    }

    return SCHEDULER_NO_ERROR;
}

void scheduler_gps_start_callback_priv(void)
{
    scheduler_event_t event;
    event.id = SCHEDULER_EVENT_GPS_START;
    scheduler_gps_callback(&event);
}

void scheduler_satpass_start_callback_priv(void)
{
    scheduler_event_t event;
    event.id = SCHEDULER_EVENT_SATPASS_START;
    scheduler_satpass_callback(&event);

    // Remove alarm once triggered (single shot)
    if (Cron.isAllocated(scheduler_alarm_satpass_start_id))
    {
        Cron.free(scheduler_alarm_satpass_start_id);
        scheduler_alarm_satpass_start_id = dtINVALID_ALARM_ID;
    }
}

int scheduler_tick(void)
{
    // DEBUG_PR_TRACE("Process EVENT... %s()", __FUNCTION__);

    Cron.delay();

    // Remove alarms in past (may happen when RTC time is not yet set)
    CronId alarm_next_trigger_id;
    time_t alarm_next_trigger_time = Cron.getNextTrigger(&alarm_next_trigger_id);
    if ((alarm_next_trigger_time < (syshal_rtc_return_timestamp() - SCHEDULER_ALARM_TIMEOUT_S)) &&
        (alarm_next_trigger_time != 0))
    {
        DEBUG_PR_WARN("Free alarm %d. Execution time in the past. %s", alarm_next_trigger_id, __FUNCTION__);
        Cron.free(alarm_next_trigger_id);
    }

    return SCHEDULER_NO_ERROR;
}

__attribute__((weak)) void scheduler_gps_callback(scheduler_event_t *event)
{
    DEBUG_PR_WARN("%s Not implemented", __FUNCTION__);
}

__attribute__((weak)) void scheduler_satpass_callback(scheduler_event_t *event)
{
    DEBUG_PR_WARN("%s Not implemented", __FUNCTION__);
}
