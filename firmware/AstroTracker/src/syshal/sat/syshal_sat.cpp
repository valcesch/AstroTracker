/******************************************************************************************
 * File:        syshal_sat.cpp
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

#include "../syshal_sat.h"
#include "../syshal_gpio.h"
#include "../syshal_time.h"
#include "../syshal_rtc.h"
#include "../../core/debug/debug.h"
#include "../syshal_config.h"

ASTRONODE astronode;

static syshal_sat_state_t state = SYSHAL_SAT_STATE_UNINIT;
static syshal_sat_config_t config;
static volatile bool new_event_pending = false;

#define SYSHAL_SAT_GPIO_INT (GPIO_ANS_EXT_INT)
#ifdef GPIO_ANS_EN
#define SYSHAL_SAT_GPIO_POWER_ON (GPIO_ANS_EN)
#endif
#ifdef GPIO_ANS_RESET
#define SYSHAL_SAT_GPIO_RESET (GPIO_ANS_RESET)
#endif
#ifdef GPIO_ANS_ANT_IN_USE
#define SYSHAL_SAT_GPIO_ANT_IN_USE (GPIO_ANS_ANT_IN_USE)
#endif

#define SYSHAL_SAT_RESTART_TIME_MS 100 //[ms] - 100ms = wakeup time from sleep mode
#define SYSHAL_SAT_RST_HTIME 1         //[ms]

#define SYSHAL_SAT_SEARCH_WINDOW_SIZE 86400            //[s]
#define SYSHAL_SAT_MAX_TLE_VALIDITY_PERIOD 365 * 86400 //[s]

// Private functions
void syshal_sat_reset_priv(void);
int syshal_sat_clear_all_messages_priv(void);
int syshal_sat_clear_performance_counter_priv(void);
int syshal_sat_get_time_priv(uint32_t *time);
int syshal_sat_read_event_priv(syshal_sat_event_id_t *event);
int syshal_sat_save_perf_counters_priv(void);
int syshal_sat_clear_reset_priv(void);
int syshal_sat_read_command_40_bytes_priv(uint8_t buffer[40],
                                          uint32_t *createdDate);
int syshal_sat_read_message_ack_priv(uint16_t *buffer_id);

static void syshal_sat_int1_pin_interrupt_priv(void)
{
    new_event_pending = true;
}

int syshal_sat_init(void)
{
    if (state == SYSHAL_SAT_STATE_ASLEEP)
        return SYSHAL_SAT_ERROR_INVALID_STATE;

    // Configure GPIOs
    syshal_gpio_init(SYSHAL_SAT_GPIO_INT, INPUT_PULLDOWN);
#ifdef SYSHAL_SAT_GPIO_POWER_ON
    syshal_gpio_init(SYSHAL_SAT_GPIO_POWER_ON, OUTPUT);
#endif
#ifdef SYSHAL_SAT_GPIO_RESET
    syshal_gpio_init(SYSHAL_SAT_GPIO_RESET, OUTPUT);
#endif
#ifdef SYSHAL_SAT_GPIO_ANT_IN_USE
    syshal_gpio_init(SYSHAL_SAT_GPIO_ANT_IN_USE, INPUT_PULLDOWN);
#endif
    syshal_gpio_enable_interrupt(SYSHAL_SAT_GPIO_INT, syshal_sat_int1_pin_interrupt_priv, CHANGE);

    // Reset module
    syshal_sat_reset_priv();

    // Try establish connection
    syshal_sat_wake_up();
    UART_ANS.begin(SYSHAL_SAT_BAUDRATE);
    if (astronode.begin(UART_ANS) != ANS_STATUS_SUCCESS)
    {
        DEBUG_PR_ERROR("Not detected at default UART port. Please check wiring. %s()", __FUNCTION__);
        syshal_sat_shutdown();
        return SYSHAL_SAT_ERROR_TIMEOUT;
    }

    // Clear all queued messages (if no reset line connected)
    syshal_sat_clear_all_messages_priv();

    // Reset internal counters
    syshal_sat_clear_performance_counter_priv();

    syshal_sat_shutdown();

    return SYSHAL_SAT_NO_ERROR;
}

int syshal_sat_update_config(syshal_sat_config_t sat_config)
{
    if (state == SYSHAL_SAT_STATE_ASLEEP)
        return SYSHAL_SAT_ERROR_INVALID_STATE;

    DEBUG_PR_TRACE("Update configuration. %s()", __FUNCTION__);

    config = sat_config;

    if (config.astronode->hdr.set)
    {
        DEBUG_PR_TRACE("Set Astronode configuration. %s()", __FUNCTION__);
        if (astronode.configuration_write(config.astronode->contents.with_pld_ack,
                                          config.astronode->contents.with_geo_loc,
                                          config.astronode->contents.with_ephemeris,
                                          config.astronode->contents.with_deep_sleep_en,
                                          config.astronode->contents.with_msg_ack_pin_en,
                                          config.astronode->contents.with_msg_reset_pin_en,
                                          config.astronode->contents.with_cmd_event_pin_en,
                                          config.astronode->contents.with_tx_pend_event_pin_en) != ANS_STATUS_SUCCESS)
        {
            return SYSHAL_SAT_ERROR_SET_CONFIGURATION;
        }

        DEBUG_PR_TRACE("Set Astronode satellite search configuration. %s()", __FUNCTION__);
        if (astronode.satellite_search_config_write(config.astronode->contents.sat_search_rate,
                                                    config.astronode->contents.sat_force_search) != ANS_STATUS_SUCCESS)
        {
            return SYSHAL_SAT_ERROR_SET_CONFIGURATION;
        }

        /*
        if (astronode.config.product_id == TYPE_WIFI_DEVKIT)
        {
            DEBUG_PR_TRACE("Write WiFi configuration. %s()", __FUNCTION__);
            astronode.wifi_configuration_write(wland_ssid,
                                               wland_key,
                                               auth_token);
        }
        */

        DEBUG_PR_TRACE("Save Astronode configuration. %s()", __FUNCTION__);
        if (astronode.configuration_save() != ANS_STATUS_SUCCESS)
        {
            return SYSHAL_SAT_ERROR_SAVE_CONFIGURATION;
        }
    }

    return SYSHAL_SAT_NO_ERROR;
}

int syshal_sat_term()
{
    astronode.end();

    return SYSHAL_SAT_NO_ERROR;
}

int syshal_sat_shutdown(void)
{
    if (state == SYSHAL_SAT_STATE_ASLEEP)
        return SYSHAL_SAT_NO_ERROR; // SAT is already shutdown

    syshal_sat_save_perf_counters_priv();

#ifdef SYSHAL_SAT_GPIO_POWER_ON
    syshal_gpio_set_output_low(SYSHAL_SAT_GPIO_POWER_ON);
#endif

    DEBUG_PR_TRACE("Shutdown. %s()", __FUNCTION__);

    state = SYSHAL_SAT_STATE_ASLEEP;

    syshal_sat_event_t event;
    event.id = SYSHAL_SAT_EVENT_POWERED_OFF;
    syshal_sat_callback(&event);

    return SYSHAL_SAT_NO_ERROR;
}

int syshal_sat_wake_up(void)
{
    if (state == SYSHAL_SAT_STATE_ACTIVE)
        return SYSHAL_SAT_NO_ERROR; // SAT is already awake

    DEBUG_PR_TRACE("Wakeup. %s()", __FUNCTION__);

#ifdef SYSHAL_SAT_GPIO_POWER_ON
    syshal_gpio_set_output_high(SYSHAL_SAT_GPIO_POWER_ON);
#endif

    syshal_time_delay_ms(SYSHAL_SAT_RESTART_TIME_MS);

    state = SYSHAL_SAT_STATE_ACTIVE;

    syshal_sat_event_t event;
    event.id = SYSHAL_SAT_EVENT_POWERED_ON;
    syshal_sat_callback(&event);

    return SYSHAL_SAT_NO_ERROR;
}

int syshal_sat_set_geolocation(int32_t lat,
                               int32_t lon)
{
    if (state == SYSHAL_SAT_STATE_ASLEEP)
        return SYSHAL_SAT_ERROR_INVALID_STATE;

    DEBUG_PR_TRACE("Set geolocation lat=%d, lon=%d. %s()", lat, lon, __FUNCTION__);

    if (astronode.geolocation_write(lat, lon) != ANS_STATUS_SUCCESS)
    {
        return SYSHAL_SAT_ERROR_SET_GEOLOCATION;
    }
    return SYSHAL_SAT_NO_ERROR;
}

int syshal_sat_send_message(uint8_t *buffer,
                            size_t buffer_size,
                            uint16_t buffer_id)
{
    if (state == SYSHAL_SAT_STATE_ASLEEP)
        return SYSHAL_SAT_ERROR_INVALID_STATE;

    ans_status_e ret_val = astronode.enqueue_payload(buffer, buffer_size, buffer_id);

    if (ret_val == ANS_STATUS_SUCCESS)
    {
        // DEBUG_PR_TRACE("Enqueue payload: NOT IMPLEMENTED. %s()", __FUNCTION__);
    }
    else if (ret_val == ANS_STATUS_BUFFER_FULL)
    {
        DEBUG_PR_TRACE("Enqueue payload: BUFFER IS FULL. %s()", __FUNCTION__);
        return SYSHAL_SAT_BUFFER_FULL;
    }

    return SYSHAL_SAT_NO_ERROR;
}

int syshal_sat_read_command_40_bytes_priv(uint8_t buffer[40],
                                          uint32_t *createdDate)
{
    if (state == SYSHAL_SAT_STATE_ASLEEP)
        return SYSHAL_SAT_ERROR_INVALID_STATE;

    if ((astronode.read_command_40B(buffer, createdDate) == ANS_STATUS_SUCCESS) &&
        (astronode.clear_command() == ANS_STATUS_SUCCESS))
    {
        // DEBUG_PR_TRACE("Read command 40bytes: NOT IMPLEMENTED. %s()", __FUNCTION__);
    }
    else
    {
        DEBUG_PR_TRACE("Read command 40bytes: COULD NOT READ COMMAND. %s()", __FUNCTION__);
        return SYSHAL_SAT_ERROR_READ_CMD;
    }

    return SYSHAL_SAT_NO_ERROR;
}

int syshal_sat_clear_all_messages_priv(void)
{
    if (state == SYSHAL_SAT_STATE_ASLEEP)
        return SYSHAL_SAT_ERROR_INVALID_STATE;

    DEBUG_PR_TRACE("Clear all messages. %s()", __FUNCTION__);

    if (astronode.clear_free_payloads() != ANS_STATUS_SUCCESS)
    {
        return SYSHAL_SAT_ERROR_CLEAR_ALL_MSG;
    }
    return SYSHAL_SAT_NO_ERROR;
}

int syshal_sat_clear_performance_counter_priv(void)
{
    if (state == SYSHAL_SAT_STATE_ASLEEP)
        return SYSHAL_SAT_ERROR_INVALID_STATE;

    DEBUG_PR_TRACE("Clear all performance counters. %s()", __FUNCTION__);

    if (astronode.clear_performance_counter() != ANS_STATUS_SUCCESS)
    {
        return SYHSAL_SAT_ERROR_CLEAR_PERF_COUNTER;
    }
    return SYSHAL_SAT_NO_ERROR;
}

int syshal_sat_read_message_ack_priv(uint16_t *buffer_id)
{
    if (state == SYSHAL_SAT_STATE_ASLEEP)
        return SYSHAL_SAT_ERROR_INVALID_STATE;

    if ((astronode.read_satellite_ack(buffer_id) == ANS_STATUS_SUCCESS) &&
        (astronode.clear_satellite_ack() == ANS_STATUS_SUCCESS))
    {
        DEBUG_PR_TRACE("Read message ACK %d. %s", *buffer_id, __FUNCTION__);
    }
    else
    {
        return SYSHAL_SAT_ERROR_CLEAR_ACK_MSG;
    }
    return SYSHAL_SAT_NO_ERROR;
}

int syshal_sat_clear_reset_priv(void)
{
    if (state == SYSHAL_SAT_STATE_ASLEEP)
        return SYSHAL_SAT_ERROR_INVALID_STATE;

    DEBUG_PR_TRACE("Clear RESET event. %s()", __FUNCTION__);

    if (astronode.clear_reset_event() != ANS_STATUS_SUCCESS)
    {
        return SYSHAL_SAT_ERROR_CLEAR_RESET;
    }
    return SYSHAL_SAT_NO_ERROR;
}

int syshal_sat_get_time_priv(uint32_t *time)
{
    if (state == SYSHAL_SAT_STATE_ASLEEP)
        return SYSHAL_SAT_ERROR_INVALID_STATE;

    if ((astronode.rtc_read(time) == ANS_STATUS_SUCCESS) &&
        (*time != ASTROCAST_REF_UNIX_TIME))
    {
        DEBUG_PR_TRACE("Time is %d. %s", *time, __FUNCTION__);
    }
    else
    {
        return SYSHAL_SAT_ERROR_READ_TIME;
    }
    return SYSHAL_SAT_NO_ERROR;
}

int syshal_sat_read_event_priv(syshal_sat_event_id_t *event)
{
    if (state == SYSHAL_SAT_STATE_ASLEEP)
        return SYSHAL_SAT_ERROR_INVALID_STATE;

    uint8_t event_tmp;
    if (astronode.event_read(&event_tmp) == ANS_STATUS_SUCCESS)
    {
        *event = (syshal_sat_event_id_t)event_tmp;

        DEBUG_PR_TRACE("Read EVENT: %d. %s()", *event, __FUNCTION__);
    }
    else
    {
        return SYSHAL_SAT_ERROR_READ_EVENT;
    }
    return SYSHAL_SAT_NO_ERROR;
}

int syshal_sat_get_next_contact_oportuinty(uint32_t *delay,
                                           syshal_sat_pwr_mode_t mode,
                                           uint32_t t_now,
                                           int32_t latitude,
                                           int32_t longitude)
{
    switch (mode)
    {
    case SYSHAL_SAT_PWR_MODE_SAT_PASS_PREDICT:
    {
        DEBUG_PR_TRACE("Contact time prediction with SAT_PASS module. %s", __FUNCTION__);
        /*
        if (satpass.read_next_contact_opportunity(t_now, delay) != SAT_NO_ERROR)
        {
            // If no satellite pass found, compute them after updating terminal location
            satpass.set_geo(latitude, longitude);

            satpass.clear_all_sat_pass_slots();
            satpass.clear_all_sat_slots();

            DEBUG_PR_TRACE("NOT IMPLEMENTED - DID NOT ADD SATELLITES TO LIST. %s", __FUNCTION__);

            for (uint8_t i = 0; i < SYSHAL_SAT_MAX_NB_SAT; i++)
            {
                satpass.add_sat_from_tle(tleName[i], tlel1[i], tlel2[i], 1, t_now + SYSHAL_SAT_MAX_TLE_VALIDITY_PERIOD);
            }

            satpass.compute_next_sat_pass(t_now, t_now + SYSHAL_SAT_SEARCH_WINDOW_SIZE);

            // Check again if new satellite passes available
            if (satpass.read_next_contact_opportunity(t_now, delay) != SAT_NO_ERROR)
            {
                return SYSHAL_SAT_ERROR_READ_NEXT_CONT_OPORT;
            }
        }*/
        break;
    }
    case SYSHAL_SAT_PWR_MODE_ASTROCAST_EPHEMERIDES:
    {
        if (state == SYSHAL_SAT_STATE_ASLEEP)
            return SYSHAL_SAT_ERROR_INVALID_STATE;

        DEBUG_PR_TRACE("Contact time prediction with Astronode S. %s", __FUNCTION__);

        if (astronode.read_next_contact_opportunity(delay) != ANS_STATUS_SUCCESS)
        {
            return SYSHAL_SAT_ERROR_READ_NEXT_CONT_OPORT;
        }

        break;
    }
    case SYSHAL_SAT_PWR_MODE_NONE:
    {
        DEBUG_PR_TRACE("No contact time prediction available. %s", __FUNCTION__);

        *delay = 0;
        break;
    }
    }

    return SYSHAL_SAT_NO_ERROR;
}

int syshal_sat_read_status(syshal_sat_status_t *status)
{
    if (state == SYSHAL_SAT_STATE_ASLEEP)
        return SYSHAL_SAT_ERROR_INVALID_STATE;

    DEBUG_PR_TRACE("Read Astronode S status. %s", __FUNCTION__);

    // Performance counters
    ASTRONODE_PER_STRUCT per_struct;
    if (astronode.read_performance_counter(&per_struct) == ANS_STATUS_SUCCESS)
    {
        status->sat_detect_operation_cnt = per_struct.sat_detect_operation_cnt;
        status->signal_demod_attempt_cnt = per_struct.signal_demod_attempt_cnt;
        status->ack_demod_attempt_cnt = per_struct.ack_demod_attempt_cnt;
        status->sent_fragment_cnt = per_struct.sent_fragment_cnt;
        status->ack_fragment_cnt = per_struct.ack_fragment_cnt;
        status->queued_msg_cnt = per_struct.queued_msg_cnt;
    }
    else
    {
        return SYSHAL_SAT_ERROR_READ_HK;
    }

    // Module state
    ASTRONODE_MST_STRUCT mst_struct;
    if (astronode.read_module_state(&mst_struct) == ANS_STATUS_SUCCESS)
    {
        status->uptime = mst_struct.uptime;
    }
    else
    {
        return SYSHAL_SAT_ERROR_READ_HK;
    }

    // Environment details
    ASTRONODE_END_STRUCT end_struct;
    if (astronode.read_environment_details(&end_struct) == ANS_STATUS_SUCCESS)
    {
        // Empty
    }
    else
    {
        return SYSHAL_SAT_ERROR_READ_HK;
    }

    // Last contact details
    ASTRONODE_LCD_STRUCT lcd_struct;
    if (astronode.read_last_contact_details(&lcd_struct) == ANS_STATUS_SUCCESS)
    {
        status->time_start_last_contact = lcd_struct.time_start_last_contact;
        status->time_end_last_contact = lcd_struct.time_end_last_contact;
        status->peak_rssi_last_contact = lcd_struct.peak_rssi_last_contact;
        status->time_peak_rssi_last_contact = lcd_struct.time_peak_rssi_last_contact;
    }
    else
    {
        return SYSHAL_SAT_ERROR_READ_HK;
    }

    return SYSHAL_SAT_NO_ERROR;
}

void syshal_sat_print_status(void)
{
}

int syshal_sat_save_perf_counters_priv(void)
{
    if (state == SYSHAL_SAT_STATE_ASLEEP)
        return SYSHAL_SAT_ERROR_INVALID_STATE;

    DEBUG_PR_TRACE("Save performance counters. %s", __FUNCTION__);

    if (astronode.save_performance_counter() != ANS_STATUS_SUCCESS)
    {
        return SYSHAL_SAT_ERROR_SAVE_PERF_COUNTERS;
    }
    return SYSHAL_SAT_NO_ERROR;
}

void syshal_sat_reset_priv(void)
{
#ifdef SYSHAL_SAT_GPIO_RESET
    syshal_gpio_set_output_high(SYSHAL_SAT_GPIO_RESET);
    syshal_time_delay_ms(SYSHAL_SAT_RST_HTIME);
    syshal_gpio_set_output_low(SYSHAL_SAT_GPIO_RESET);
#else
    DEBUG_PR_ERROR("No RESET pin connected. %s", __FUNCTION__);
#endif
}

syshal_sat_state_t syshal_sat_get_state(void)
{
    return state;
}

int syshal_sat_tick(void)
{
    if (state == SYSHAL_SAT_STATE_UNINIT)
        return SYSHAL_SAT_ERROR_INVALID_STATE;

    if (new_event_pending) // syshal_gpio_get_input(SYSHAL_SAT_GPIO_INT)
    {
        syshal_sat_wake_up();

        syshal_sat_event_t event;
        syshal_sat_event_id_t event_id;

        while (syshal_sat_read_event_priv(&event_id) == SYSHAL_SAT_NO_ERROR)
        {
            DEBUG_PR_TRACE("Process EVENT... %s()", __FUNCTION__);

            switch (event_id)
            {
            case SYSHAL_SAT_EVENT_MSG_ACK:
            {
                syshal_sat_read_message_ack_priv(&event.msg_acknowledged.msg_id);
                event.id = SYSHAL_SAT_EVENT_MSG_ACK;
                syshal_rtc_get_timestamp(&event.msg_acknowledged.timestamp);
                DEBUG_PR_TRACE("Got MSG ACKNOWLEDGED. %s()", __FUNCTION__);
                syshal_sat_callback(&event);
                break;
            }
            case SYSHAL_SAT_EVENT_COMMAND_RECEIVED:
            {
                syshal_sat_read_command_40_bytes_priv(event.cmd_received.buffer, &event.cmd_received.createdDate);
                event.id = SYSHAL_SAT_EVENT_COMMAND_RECEIVED;
                event.cmd_received.buffer_size = 40;
                syshal_rtc_get_timestamp(&event.cmd_received.timestamp);
                DEBUG_PR_TRACE("Got CMD RECEIVED. %s()", __FUNCTION__);
                syshal_sat_callback(&event);
                break;
            }
            case SYSHAL_SAT_EVENT_RESET:
            {
                syshal_sat_clear_reset_priv();
                event.id = SYSHAL_SAT_EVENT_RESET;
                DEBUG_PR_TRACE("Got RESET. %s()", __FUNCTION__);
                syshal_sat_callback(&event);
                break;
            }
            case SYSHAL_SAT_EVENT_NO_EVENT:
            {
                event.id = SYSHAL_SAT_EVENT_NO_EVENT;
                DEBUG_PR_TRACE("Got NO EVENT. %s()", __FUNCTION__);
                syshal_sat_callback(&event);
                break;
            }
            case SYSHAL_SAT_EVENT_MESSAGE_PENDING:
            {
                event.id = SYSHAL_SAT_EVENT_MESSAGE_PENDING;
                DEBUG_PR_TRACE("Got MSG PENDING. %s()", __FUNCTION__);
                syshal_sat_callback(&event);
                break;
            }
            default:
                break;
            }

            if ((event_id == SYSHAL_SAT_EVENT_NO_EVENT) ||
                (event_id == SYSHAL_SAT_EVENT_MESSAGE_PENDING))
                break;
        }

        syshal_sat_shutdown();
        new_event_pending = false;
    }

    return SYSHAL_SAT_NO_ERROR;
}

__attribute__((weak)) void syshal_sat_callback(syshal_sat_event_t *event)
{
    DEBUG_PR_WARN("%s Not implemented", __FUNCTION__);
}
