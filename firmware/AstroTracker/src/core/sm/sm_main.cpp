/******************************************************************************************
 * File:        sm_main.cpp
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

#include "sm_main.h"
#include <Wire.h>
#include "../config/sys_config.h"
#include "../debug/debug.h"
#include "../scheduler/scheduler.h"
#include "../config/version.h"
#include "../logger/logger.h"
#include "../command/an_command.h"
#include "../loopbackstream/LoopbackStream.h"
#include "../../syshal/syshal_rtc.h"
#include "../../syshal/syshal_pmu.h"
#include "../../syshal/syshal_led.h"
#include "../../syshal/syshal_batt.h"
#include "../../syshal/syshal_gps.h"
#include "../../syshal/syshal_temp.h"
#include "../../syshal/syshal_sat.h"
#include "../../syshal/syshal_ble.h"
#include "../../syshal/syshal_gpio.h"

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// MAIN STATES ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static void sm_main_boot(sm_handle_t *state_handle);
static void sm_main_error(sm_handle_t *state_handle);
static void sm_main_battery_level_low(sm_handle_t *state_handle);
static void sm_main_log_file_full(sm_handle_t *state_handle);
static void sm_main_provisioning(sm_handle_t *state_handle);
static void sm_main_operational(sm_handle_t *state_handle);
static void sm_main_deactivate(sm_handle_t *state_handle);

sm_state_func_t sm_main_states[] =
    {
        [SM_MAIN_BOOT] = sm_main_boot,
        [SM_MAIN_ERROR] = sm_main_error,
        [SM_MAIN_BATTERY_LEVEL_LOW] = sm_main_battery_level_low,
        [SM_MAIN_LOG_FILE_FULL] = sm_main_log_file_full,
        [SM_MAIN_PROVISIONING] = sm_main_provisioning,
        [SM_MAIN_OPERATIONAL] = sm_main_operational,
        [SM_MAIN_DEACTIVATE] = sm_main_deactivate,
};

static const char *sm_main_state_str[] =
    {
        [SM_MAIN_BOOT] = "SM_MAIN_BOOT",
        [SM_MAIN_ERROR] = "SM_MAIN_ERROR",
        [SM_MAIN_BATTERY_LEVEL_LOW] = "SM_MAIN_BATTERY_LEVEL_LOW",
        [SM_MAIN_LOG_FILE_FULL] = "SM_MAIN_LOG_FILE_FULL",
        [SM_MAIN_PROVISIONING] = "SM_MAIN_PROVISIONING",
        [SM_MAIN_OPERATIONAL] = "SM_MAIN_OPERATIONAL",
        [SM_MAIN_DEACTIVATE] = "SM_MAIN_DEACTIVATE",
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// GLOBALS /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define LED_BLINK_FAIL_DURATION_MS (100)
#define LED_DURATION_MS (3000)
#define LED_BLINK_TEST_PASSED_DURATION_MS (2 * LED_BLINK_FAIL_DURATION_MS)
#define LED_DEACTIVATED_STATE_DURATION_MS (6000)

#define SOFT_WATCHDOG_TIMEOUT_S (16)   // How many seconds to allow before soft watchdog trips -> DO NOT SET LOWER THAN 16S
#define HARD_WATCHDOG_TIMEOUT_S (3600) // How many seconds to allow before soft watchdog trips -> WILL DEPEND ON HARDWARE CONFIGURATION

#define GPS_ACTIVE_WAKEUP_TIMEOUT_S (3)

#define RTC_DEFAULT_TIMESTAMP_S (1514764740 + 50) // Sun Dec 31 2017 23:59:00 GMT+0000

#define KICK_WATCHDOG() syshal_rtc_soft_watchdog_refresh()

static uint8_t last_battery_reading;
static volatile bool sensor_logging_enabled = false; // Are sensors currently allowed to log
static volatile bool logger_new_data_available = false;
static volatile bool new_config_available = false;

static uint32_t gps_start_time;
static uint32_t sat_start_time;
static uint32_t ble_start_time;
static uint32_t led_finish_time;

COMMAND syshal_ble_command;
COMMAND syshal_sat_command;
LoopbackStream sat_stream;
LoopbackStream ble_stream;

typedef struct __attribute__((__packed__))
{
    uint32_t timestamp;
    int32_t lat,
        lon;
    uint8_t SIV;
    int32_t gSpeed;
    uint8_t v_bat;
    int8_t temp;
} LOG_PVT_struct;

typedef struct __attribute__((__packed__))
{
    uint8_t data[40];
} LOG_U_MSG_struct;

typedef struct __attribute__((__packed__))
{
    uint8_t data[40];
} LOG_U_CMD_struct;

typedef struct __attribute__((__packed__))
{
    uint8_t meas20[20];
} LOG_RAW_struct;

#define LOGGER_TAG_PVT_SLOT (0x01)
#define LOGGER_TAG_U_MSG_SLOT (0x02)
#define LOGGER_TAG_U_CMD_SLOT (0x03)
#define LOGGER_TAG_RAW_SLOT (0x04)

typedef struct
{
    struct
    {
        syshal_sat_status_t status;
        uint16_t reset_cnt = 0;
        uint32_t uptime = 0;
    } sat_counters;

    struct
    {
        uint16_t u_msg_cnt = 0;
        uint16_t u_cmd_cnt = 0;
        uint16_t pvt_cnt = 0;
        uint16_t raw_cnt = 0;
        // uint32_t total_mem_size = 0;
    } logger_counters;

    struct
    {
        uint16_t meas_cnt = 0;
        int32_t last_loc_lat = 0;
        int32_t last_loc_lon = 0;
        uint32_t time_last_update = 0;
        uint32_t uptime = 0;
    } gps_counters;

    struct
    {
        uint32_t advert_burst_cnt = 0;
        uint16_t user_connection_cnt = 0;
        uint32_t uptime = 0;
    } ble_counters;

    struct
    {
        uint32_t sleep_time = 0;
        uint32_t up_time = 0;
    } asset_counters;
} sm_context_t;
static sm_context_t sm_context;

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// PROTOTYPES ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static bool check_configuration_tags_set(void);
void ble_write_req(void);
void logger_push_slots_to_sat(void);
void state_message_exception_handler(CEXCEPTION_T e);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// CALLBACK FUNCTIONS //////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void syshal_gps_callback(syshal_gps_event_t *event)
{
    DEBUG_PR_TRACE("%s() called", __FUNCTION__);

    switch (event->id)
    {
    case SYSHAL_GPS_EVENT_POWERED_ON:
        DEBUG_PR_TRACE("SYSHAL_GPS_EVENT_POWERED_ON");
        sm_context.gps_counters.meas_cnt++;
        gps_start_time = syshal_rtc_return_uptime();
        break;
    case SYSHAL_GPS_EVENT_POWERED_OFF:
        DEBUG_PR_TRACE("SYSHAL_GPS_EVENT_POWERED_OFF");
        sm_context.gps_counters.uptime += syshal_rtc_return_uptime() - gps_start_time;
        break;
    case SYSHAL_GPS_EVENT_STATUS:
    {
        DEBUG_PR_TRACE("SYSHAL_GPS_EVENT_STATUS - iTOW: %d, gpsFix: %d, flags: %d, fixStat: %d, flags2: %d, ttf: %d, msss: %d.",
                       event->status.iTOW,
                       event->status.gpsFix,
                       event->status.flags,
                       event->status.fixStat,
                       event->status.flags2,
                       event->status.ttff,
                       event->status.msss);
        break;
    }
    case SYSHAL_GPS_EVENT_PVT:
    {
        DEBUG_PR_TRACE("SYSHAL_GPS_EVENT_PVT - %d fix: %d, lat: %d, lon: %d.",
                       event->pvt.timestamp,
                       event->pvt.gpsFix,
                       event->pvt.lat,
                       event->pvt.lon);

        // Update RTC time
        DEBUG_PR_TRACE("Update RTC from GPS.");
        syshal_rtc_set_timestamp(event->pvt.timestamp);

        // Store data
        uint16_t slot_id = 0;
        LOG_PVT_struct log_pvt;
        log_pvt.timestamp = event->pvt.timestamp;
        log_pvt.lat = event->pvt.lat;
        log_pvt.lon = event->pvt.lon;
        log_pvt.SIV = event->pvt.SIV;
        log_pvt.gSpeed = event->pvt.gSpeed;

        sm_context.gps_counters.last_loc_lat = event->pvt.lat;
        sm_context.gps_counters.last_loc_lon = event->pvt.lon;
        sm_context.gps_counters.time_last_update = event->pvt.timestamp;

        syshal_temp_temperature(&log_pvt.temp);
        syshal_batt_voltage(&log_pvt.v_bat);
        logger_insert_data(&log_pvt, sizeof(LOG_PVT_struct), LOGGER_TAG_PVT_SLOT,
                           syshal_rtc_return_timestamp(), &slot_id);

        sm_context.logger_counters.pvt_cnt++;
        logger_new_data_available = true;
        break;
    }
    case SYSHAL_GPS_EVENT_RAW:
    {
        DEBUG_PR_TRACE("SYSHAL_GPS_EVENT_RAW");

        if ((sys_config.gps_settings.hdr.set) &&
            (sys_config.gps_settings.contents.with_rxm_meas20) &&
            ((syshal_rtc_return_uptime() - gps_start_time) <= sys_config.gps_settings.contents.raw_timeout_s))
        {
            // Store data
            uint16_t slot_id = 0;
            LOG_RAW_struct log_raw;
            memcpy(log_raw.meas20, event->raw.meas20, 20);

            logger_insert_data(&log_raw, sizeof(LOG_RAW_struct), LOGGER_TAG_RAW_SLOT,
                               syshal_rtc_return_timestamp(), &slot_id);

            sm_context.logger_counters.raw_cnt++;
            logger_new_data_available = true;
        }
        break;
    }
    default:
        DEBUG_PR_WARN("Unknown GPS event in %s() : %d", __FUNCTION__, event->id);
        break;
    }
}

void scheduler_gps_callback(scheduler_event_t *event)
{
    DEBUG_PR_TRACE("%s() called", __FUNCTION__);

    switch (event->id)
    {
    case SCHEDULER_EVENT_GPS_START:
        syshal_gps_wake_up();
        break;
    case SCHEDULER_EVENT_GPS_TIMEOUT:
        syshal_gps_shutdown();
        break;
    default:
        DEBUG_PR_WARN("Unknown SCHEDULER event in %s() : %d", __FUNCTION__, event->id);
        break;
    }
}

void syshal_sat_callback(syshal_sat_event_t *event)
{
    DEBUG_PR_TRACE("%s() called", __FUNCTION__);

    switch (event->id)
    {
    case SYSHAL_SAT_EVENT_POWERED_ON:
        DEBUG_PR_TRACE("SYSHAL_SAT_EVENT_POWERED_ON");
        sat_start_time = syshal_rtc_return_uptime();
        break;
    case SYSHAL_SAT_EVENT_POWERED_OFF:
        DEBUG_PR_TRACE("SYSHAL_SAT_EVENT_POWERED_OFF");
        sm_context.sat_counters.uptime += syshal_rtc_return_uptime() - sat_start_time;
        break;
    case SYSHAL_SAT_EVENT_MSG_ACK:
    {
        DEBUG_PR_TRACE("SYSHAL_SAT_EVENT_MSG_ACK");

        uint8_t slot_tag;
        logger_set_status_of_slot_id(event->msg_acknowledged.msg_id, LOGGER_SLOT_STATUS_TRANSMITTED);
        logger_set_acknowledgeddate_of_slot_id(event->msg_acknowledged.msg_id, event->msg_acknowledged.timestamp);
        logger_get_tag_from_slot_id(event->msg_acknowledged.msg_id, &slot_tag);
        if ((slot_tag == LOGGER_TAG_PVT_SLOT) ||
            (slot_tag == LOGGER_TAG_RAW_SLOT)) // We keep all other slots
            logger_clear_slot(event->msg_acknowledged.msg_id);
        break;
    }
    case SYSHAL_SAT_EVENT_RESET:
        DEBUG_PR_TRACE("SYSHAL_SAT_EVENT_RESET");
        sm_context.sat_counters.reset_cnt++;
        break;
    case SYSHAL_SAT_EVENT_COMMAND_RECEIVED:
        DEBUG_PR_TRACE("SYSHAL_SAT_EVENT_COMMAND_RECEIVED");

        // Write data to virtual stream
        sat_stream.write(event->cmd_received.buffer, event->cmd_received.buffer_size);

        // Decode command and process actions
        uint8_t an_packet_id;
        if (syshal_sat_command.request_is_available() &&
            !syshal_sat_command.an_packet_receive_id(&an_packet_id))
        {
            DEBUG_PR_TRACE("New command ID = %s.", packet_id_str[an_packet_id]);

            switch (an_packet_id)
            {
            case packet_id_msg_data:
            {
                DEBUG_PR_TRACE("Receive user message data.");
                LOG_U_MSG_struct log_u_cmd;
                uint16_t slot_id = 0;
                if (!syshal_sat_command.receive_msg_data_packet(log_u_cmd.data))
                {
                    logger_insert_data(&log_u_cmd, sizeof(LOG_U_CMD_struct), LOGGER_TAG_U_CMD_SLOT,
                                       event->cmd_received.createdDate, &slot_id);
                    logger_set_acknowledgeddate_of_slot_id(slot_id, event->cmd_received.timestamp);
                    logger_set_status_of_slot_id(slot_id, LOGGER_SLOT_STATUS_TRANSMITTED);
                    sm_context.logger_counters.u_cmd_cnt++;
                }
                break;
            }
            case packet_id_config:
            {
                DEBUG_PR_TRACE("Receive new configuration.");

                uint8_t scheduler_log_data_rate;
                uint8_t scheduler_gnss_pvt_retry_rate;
                uint8_t scheduler_gnss_raw_retry_rate;
                uint8_t scheduler_keep_alive_rate;
                uint8_t scheduler_gnss_pvt_retry_count;
                uint8_t scheduler_gnss_raw_retry_count;
                uint8_t terminal_sat_search_rate;
                int32_t asset_latitude;
                int32_t asset_longitude;
                uint16_t asset_interface_enabled;
                uint8_t asset_power_saving;

                if (!syshal_sat_command.receive_config_packet(&scheduler_log_data_rate,
                                                              &scheduler_gnss_pvt_retry_rate,
                                                              &scheduler_gnss_raw_retry_rate,
                                                              &scheduler_keep_alive_rate,
                                                              &scheduler_gnss_pvt_retry_count,
                                                              &scheduler_gnss_raw_retry_count,
                                                              &terminal_sat_search_rate,
                                                              &asset_latitude,
                                                              &asset_longitude,
                                                              &asset_interface_enabled,
                                                              &asset_power_saving))
                {
                    // Save tracker configuration (TODO: Save to file system)
                    sys_config.sat_settings.contents.sat_search_rate = terminal_sat_search_rate;

                    sys_config.scheduler_settings.contents.gps_interval_h = scheduler_log_data_rate;
                    sys_config.scheduler_settings.contents.gps_timeout_s = 120;

                    new_config_available = true;
                }
            }
            default:
                break;
            }

            // Clear buffer, ready for new command
            sat_stream.clear();
        }
        break;

    case SYSHAL_SAT_EVENT_MESSAGE_PENDING:
        DEBUG_PR_TRACE("SYSHAL_SAT_EVENT_MESSAGE_PENDING");
        break;

    default:
        DEBUG_PR_WARN("Unknown SAT event in %s() : %d", __FUNCTION__, event->id);
        break;
    }
}

void syshal_ble_callback(syshal_ble_event_t *event)
{
    DEBUG_PR_TRACE("%s() called", __FUNCTION__);

    switch (event->id)
    {
    case SYSHAL_BLE_EVENT_CONNECTED:
        sm_context.ble_counters.user_connection_cnt++;
        ble_start_time = syshal_rtc_return_uptime();
        break;
    case SYSHAL_BLE_EVENT_DISCONNECTED:
        sm_context.ble_counters.uptime += syshal_rtc_return_uptime() - ble_start_time;
        break;
    case SYSHAL_BLE_EVENT_START_ADVERTISING:
        // Empty
        break;
    case SYSHAL_BLE_EVENT_COMMAND_RECEIVED:
        // Write data to virtual stream
        ble_stream.write(event->cmd_received.buffer, event->cmd_received.buffer_size);

        // Decode command and process actions
        uint8_t an_packet_id;
        if (syshal_ble_command.request_is_available() &&
            !syshal_ble_command.an_packet_receive_id(&an_packet_id))
        {
            DEBUG_PR_TRACE("New command ID = %s.", packet_id_str[an_packet_id]);

            switch (an_packet_id)
            {
            case packet_id_msg_data:
            {
                DEBUG_PR_TRACE("Receive user message data.");
                LOG_U_MSG_struct log_u_msg;
                uint16_t slot_id = 0;
                if (!syshal_ble_command.receive_msg_data_packet(log_u_msg.data))
                {
                    logger_insert_data(&log_u_msg, sizeof(LOG_U_MSG_struct), LOGGER_TAG_U_MSG_SLOT,
                                       syshal_rtc_return_timestamp(), &slot_id);
                    sm_context.logger_counters.u_msg_cnt++;
                    logger_new_data_available = true;
                    ble_write_req();
                }
                break;
            }
            case packet_id_request:
            {
                uint8_t request_id = 0;
                if (!syshal_ble_command.receive_request_packet(&request_id))
                {
                    DEBUG_PR_TRACE("New Request ID = %s.", packet_id_str[request_id]);
                    ble_write_req();

                    switch (request_id)
                    {
                    case packet_id_terminal_status:
                    {
                        DEBUG_PR_TRACE("Send asset status report.");
                        syshal_ble_command.send_terminal_status_packet(sm_context.sat_counters.status.queued_msg_cnt,
                                                                       sm_context.sat_counters.status.ack_fragment_cnt,
                                                                       0,
                                                                       sm_context.sat_counters.status.uptime,
                                                                       0,
                                                                       sm_context.sat_counters.status.peak_rssi_last_contact,
                                                                       sm_context.sat_counters.status.time_peak_rssi_last_contact,
                                                                       syshal_rtc_return_timestamp(),
                                                                       0,
                                                                       0,
                                                                       sm_context.gps_counters.last_loc_lat,
                                                                       sm_context.gps_counters.last_loc_lon,
                                                                       sm_context.gps_counters.time_last_update);
                        ble_write_req();
                        break;
                    }
                    case packet_id_clear_msg_data:
                        DEBUG_PR_TRACE("Clear all messages in logger.");
                        syshal_ble_command.send_clear_msg_data_packet();
                        ble_write_req();
                        logger_clear_all_slots_matching_tag(LOGGER_TAG_U_MSG_SLOT);
                        break;
                    case packet_id_update_loc_data:
                        DEBUG_PR_TRACE("Trig sensors measurement.");
                        syshal_ble_command.send_update_loc_data_packet();
                        ble_write_req();
                        syshal_gps_wake_up();
                        break;
                    case packet_id_cmd_data:
                    {
                        DEBUG_PR_TRACE("Get all user CMDs from logger.");
                        LOG_U_CMD_struct log_u_cmd;
                        uint32_t prv_slot_epoch = 0xFFFFFFFF;
                        for (uint16_t i = 0; i < LOGGER_NB_SLOTS; i++)
                        {
                            uint16_t slot_id = 0, slot_size = 0;
                            uint32_t slot_createdDate = 0, slot_acknowledgedDate = 0;
                            uint8_t slot_tag;
                            logger_slot_status_id_t status;
                            void *slot_buffer;

                            if (!logger_get_youngest_slot_id_older_than(&slot_id, &slot_createdDate, prv_slot_epoch))
                            {
                                logger_get_tag_from_slot_id(slot_id, &slot_tag);
                                if (slot_tag == LOGGER_TAG_U_CMD_SLOT)
                                {
                                    logger_get_data(slot_id, &slot_buffer, &slot_size, &slot_tag, &slot_createdDate, &slot_acknowledgedDate, &status);
                                    memcpy(&log_u_cmd, slot_buffer, slot_size);
                                    syshal_ble_command.send_msg_data_packet(log_u_cmd.data, slot_createdDate);
                                    ble_write_req();
                                }
                            }
                            prv_slot_epoch = slot_createdDate;
                        }
                        break;
                    }
                    case packet_id_msg_data:
                    {
                        DEBUG_PR_TRACE("Get all user MSGs from logger.");
                        LOG_U_MSG_struct log_u_msg;
                        uint32_t prv_slot_epoch = 0xFFFFFFFF;
                        for (uint16_t i = 0; i < LOGGER_NB_SLOTS; i++)
                        {
                            uint16_t slot_id = 0, slot_size = 0;
                            uint32_t slot_createdDate = 0, slot_acknowledgedDate = 0;
                            uint8_t slot_tag;
                            logger_slot_status_id_t status;
                            void *slot_buffer;

                            if (!logger_get_youngest_slot_id_older_than(&slot_id, &slot_createdDate, prv_slot_epoch))
                            {
                                logger_get_tag_from_slot_id(slot_id, &slot_tag);
                                if (slot_tag == LOGGER_TAG_U_MSG_SLOT)
                                {
                                    logger_get_data(slot_id, &slot_buffer, &slot_size, &slot_tag, &slot_createdDate, &slot_acknowledgedDate, &status);
                                    memcpy(&log_u_msg, slot_buffer, slot_size);
                                    syshal_ble_command.send_msg_data_packet(log_u_msg.data, slot_acknowledgedDate);
                                    ble_write_req();
                                }
                            }
                            prv_slot_epoch = slot_createdDate;
                        }
                        break;
                    }
                    default:
                        DEBUG_PR_WARN("Request not supported.");
                        break;
                    }
                }
                break;
            }
            default:
                DEBUG_PR_WARN("Packet not supported.");
                break;
            }

            // Clear buffer, ready for new command
            ble_stream.clear();
        }
        break;
    default:
        DEBUG_PR_WARN("Unknown BLE event in %s() : %d", __FUNCTION__, event->id);
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////
///////////////////////////// STATE EXECUTION CODE /////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static void sm_main_boot(sm_handle_t *state_handle)
{
    CEXCEPTION_T e = CEXCEPTION_NONE;

    Try
    {
        if (debug_init())
            Throw(EXCEPTION_BOOT_ERROR);

        syshal_pmu_init();

        if (syshal_rtc_init())
            Throw(EXCEPTION_BOOT_ERROR);

        if (syshal_time_init())
            Throw(EXCEPTION_BOOT_ERROR);

        Wire.begin(); // Init I2C port

        /*
        // Init timers
        int error = 0;
        error |= syshal_timer_init(&timer_gps_test_fix_hold_time, timer_gps_test_fix_hold_time_callback);
        error |= syshal_timer_init(&timer_log_flush, timer_log_flush_callback);
        error |= syshal_timer_init(&timer_saltwater_switch_hysteresis, timer_saltwater_switch_hysteresis_callback);
        error |= syshal_timer_init(&timer_pressure_interval, timer_pressure_interval_callback);
        error |= syshal_timer_init(&timer_pressure_maximum_acquisition, timer_pressure_maximum_acquisition_callback);
        error |= syshal_timer_init(&timer_axl_interval, timer_axl_interval_callback);
        error |= syshal_timer_init(&timer_axl_maximum_acquisition, timer_axl_maximum_acquisition_callback);
        error |= syshal_timer_init(&timer_ble_interval, timer_ble_interval_callback);
        error |= syshal_timer_init(&timer_ble_duration, timer_ble_duration_callback);
        error |= syshal_timer_init(&timer_ble_timeout, timer_ble_timeout_callback);
        error |= syshal_timer_init(&timer_reed_switch_timeout, timer_reed_switch_timeout_callback);
        if (error)
            Throw(EXCEPTION_BOOT_ERROR);
        */

        if (syshal_led_init())
            Throw(EXCEPTION_BOOT_ERROR);

        if (syshal_batt_init())
            Throw(EXCEPTION_BOOT_ERROR);

        if (syshal_temp_init())
            Throw(EXCEPTION_BOOT_ERROR);

        if (syshal_gps_init())
            Throw(EXCEPTION_BOOT_ERROR);

        if (syshal_sat_init())
            Throw(EXCEPTION_BOOT_ERROR);
        syshal_sat_command.begin(sat_stream, false); // Loopback data stream to command module

        if (syshal_ble_init())
            Throw(EXCEPTION_BOOT_ERROR);
        syshal_ble_command.begin(ble_stream, true); // Loopback data stream to command module

        if (logger_init())
            Throw(EXCEPTION_BOOT_ERROR);

        if (scheduler_init())
            Throw(EXCEPTION_BOOT_ERROR);

        // Start the soft watchdog timer
        // if (syshal_rtc_soft_watchdog_set(SOFT_WATCHDOG_TIMEOUT_S))
        //     Throw(EXCEPTION_BOOT_ERROR);
        // syshal_rtc_soft_watchdog_enable();
        syshal_rtc_set_timestamp(RTC_DEFAULT_TIMESTAMP_S);

        // Load tracker configuration (TODO: load from file system)
        sys_config.ble_settings.contents.tx_power = 0;
        sys_config.ble_settings.contents.advert_fast_interval = 32;
        sys_config.ble_settings.contents.advert_slow_interval = 2056; // 152.5 ms, 211.25 ms, 318.75 ms, 417.5 ms, 546.25 ms, 760 ms, 852.5 ms, 1022.5 ms, 1285 ms -> Set Interval in unit of 0.625 ms
        sys_config.ble_settings.contents.advert_fast_timeout = 30;    // [s]
        sys_config.ble_settings.hdr.set = true;

        sys_config.gps_settings.contents.with_gps = true;
        sys_config.gps_settings.contents.with_galileo = true;
        sys_config.gps_settings.contents.with_beidou = true;
        sys_config.gps_settings.contents.with_glonass = true;
        sys_config.gps_settings.contents.with_rxm_meas20 = true;
        sys_config.gps_settings.contents.raw_timeout_s = 10;
        sys_config.gps_settings.hdr.set = true;

        sys_config.sat_settings.contents.with_pld_ack = true;
        sys_config.sat_settings.contents.with_geo_loc = false;
        sys_config.sat_settings.contents.with_ephemeris = true;
        sys_config.sat_settings.contents.with_deep_sleep_en = false;
        sys_config.sat_settings.contents.with_msg_ack_pin_en = true;
        sys_config.sat_settings.contents.with_msg_reset_pin_en = false;
        sys_config.sat_settings.contents.with_cmd_event_pin_en = true;
        sys_config.sat_settings.contents.with_tx_pend_event_pin_en = false;
        sys_config.sat_settings.contents.sat_search_rate = 0;
        sys_config.sat_settings.contents.sat_force_search = false;
        // sys_config.sat_settings.contents.bulletin_data[8]; // Satellite keplerian orbital information
        sys_config.sat_settings.hdr.set = true;

        sys_config.scheduler_settings.contents.gps_interval_h = 3;
        sys_config.scheduler_settings.contents.gps_timeout_s = 120;
        sys_config.scheduler_settings.hdr.set = true;

        sys_config.battery_low_threshold.contents.threshold = 0;
        sys_config.battery_low_threshold.hdr.set = false;

        sys_config.gps_log_position_enable.contents.enable = true;
        sys_config.gps_log_position_enable.hdr.set = true;

        // Print General System Info
        DEBUG_PR_SYS("AstroTracker");
        DEBUG_PR_SYS("Compiled: %s %s With %s", COMPILE_DATE, COMPILE_TIME, COMPILER_NAME);
        DEBUG_PR_SYS("Startup/Reset reason 0x%08lX", syshal_pmu_get_startup_status());

        bool ready_for_provisionning_state = check_configuration_tags_set();

        if (ready_for_provisionning_state)
        {
            sm_set_next_state(state_handle, SM_MAIN_PROVISIONING);
        }
        else
        {
            sm_set_next_state(state_handle, SM_MAIN_ERROR);
        }

        if (sm_is_last_entry(state_handle))
        {
            syshal_led_off();
        }
    }
    Catch(e)
    {
        state_message_exception_handler(e);
        sm_set_next_state(state_handle, SM_MAIN_ERROR);
        return;
    }
}

static void sm_main_provisioning(sm_handle_t *state_handle)
{
    CEXCEPTION_T e = CEXCEPTION_NONE;

    Try
    {
        KICK_WATCHDOG();

        if (sm_is_first_entry(state_handle))
        {
            DEBUG_PR_INFO("Entered state %s from %s",
                          sm_main_state_str[sm_get_current_state(state_handle)],
                          sm_main_state_str[sm_get_last_state(state_handle)]);

            // Led for showing it enters in provisionning state
            led_finish_time = syshal_time_get_ticks_ms() + LED_DURATION_MS;
            syshal_led_set_sequence(RED_GREEN_BLUE, LED_BLINK_TEST_PASSED_DURATION_MS);

            // Branch to Battery Low state if battery is beneath threshold
            uint8_t level;
            if (!syshal_batt_level(&level))
                if (sys_config.battery_low_threshold.hdr.set &&
                    level <= sys_config.battery_low_threshold.contents.threshold)
                    sm_set_next_state(state_handle, SM_MAIN_BATTERY_LEVEL_LOW);

            // Configure GPS
            if (!syshal_gps_wake_up())
            {
                syshal_gps_config_t gps_config = {.gps = &sys_config.gps_settings};
                if (syhsal_gps_update_config(gps_config))
                    Throw(EXCEPTION_GPS_ERROR);
                syshal_gps_shutdown();
            }
            else
                Throw(EXCEPTION_GPS_ERROR);

            // Configure satellite
            if (!syshal_sat_wake_up())
            {
                syshal_sat_config_t sat_config = {.astronode = &sys_config.sat_settings};
                if (syshal_sat_update_config(sat_config))
                    Throw(EXCEPTION_SAT_ERROR);
                syshal_sat_shutdown();
            }
            else
                Throw(EXCEPTION_SAT_ERROR);
            syshal_sat_tick(); // Process reset event

            // Configure BLE radio
            syshal_ble_config_t ble_config = {.ble = &sys_config.ble_settings};
            if (syshal_ble_update_config(ble_config))
                Throw(EXCEPTION_BLE_ERROR);

            // Configure scheduler
            scheduler_config_t scheduler_config = {.scheduler = &sys_config.scheduler_settings};
            if (scheduler_update_config(scheduler_config))
                Throw(EXCEPTION_SCHEDULER_ERROR);
            scheduler_tick();
        }

        // Turn off led after led_finish_time
        if (syshal_led_is_active())
        {
            uint32_t current_time = syshal_time_get_ticks_ms();
            if (led_finish_time != 0 && current_time > led_finish_time)
            {
                syshal_led_off();
                sm_set_next_state(state_handle, SM_MAIN_OPERATIONAL);
            }
        }

        syshal_led_tick();

        if (sm_is_last_entry(state_handle))
        {
            new_config_available = false;
            syshal_led_off();
        }
    }
    Catch(e)
    {
        state_message_exception_handler(e);
        sm_set_next_state(state_handle, SM_MAIN_ERROR);
        return;
    }
}

static void sm_main_operational(sm_handle_t *state_handle)
{
    CEXCEPTION_T e = CEXCEPTION_NONE;

    Try
    {
        KICK_WATCHDOG();

        if (sm_is_first_entry(state_handle))
        {
            DEBUG_PR_INFO("Entered state %s from %s",
                          sm_main_state_str[sm_get_current_state(state_handle)],
                          sm_main_state_str[sm_get_last_state(state_handle)]);

            if (sys_config.logging_enable.hdr.set &&
                sys_config.logging_enable.contents.enable)
                sensor_logging_enabled = true;

            // Led for showing it enters in operational state
            led_finish_time = syshal_time_get_ticks_ms() + LED_DURATION_MS;
            syshal_led_set_blinking(SYSHAL_LED_COLOUR_GREEN, LED_BLINK_TEST_PASSED_DURATION_MS);

            // If GPS logging enabled
            if ((sys_config.gps_log_position_enable.hdr.set &&
                 sys_config.gps_log_position_enable.contents.enable))
            {
                scheduler_gps_start();
            }
            else
            {
                syshal_gps_shutdown();
            }
        }

        // Get the battery level state
        uint8_t level;
        if (!syshal_batt_level(&level))
        {
            // Has our battery level decreased
            if (last_battery_reading > level)
            {
                // Should we check to see if we should enter a low power state?
                if (sys_config.battery_low_threshold.hdr.set &&
                    level <= sys_config.battery_low_threshold.contents.threshold)
                    sm_set_next_state(state_handle, SM_MAIN_BATTERY_LEVEL_LOW);

                last_battery_reading = level;
            }
        }

        if ((sys_config.gps_log_position_enable.hdr.set &&
             sys_config.gps_log_position_enable.contents.enable))
        {
            if (!syshal_led_is_active())
            {
                syshal_led_set_solid(SYSHAL_LED_COLOUR_GREEN);
                syshal_led_tick();
            }

            syshal_gps_tick();

            // If we have a raw sample aquired
            if ((sys_config.gps_settings.hdr.set) &&
                (sys_config.gps_settings.contents.with_rxm_meas20) &&
                ((syshal_rtc_return_uptime() - gps_start_time) <= sys_config.gps_settings.contents.raw_timeout_s) &&
                (syshal_gps_get_state() == SYSHAL_GPS_STATE_FIXED_RAW))
                syshal_gps_shutdown();

            // If we have a 3D fix
            if (syshal_gps_get_state() == SYSHAL_GPS_STATE_FIXED)
                syshal_gps_shutdown();

            // If we have a timeout
            if ((syshal_rtc_return_uptime() - gps_start_time) > sys_config.scheduler_settings.contents.gps_timeout_s)
                syshal_gps_shutdown();
        }

        // Turn off led after led_finish_time
        if (syshal_led_is_active())
        {
            uint32_t current_time = syshal_time_get_ticks_ms();

            // If there a no finish time or the current time is less than the finish time
            if (led_finish_time != 0 && current_time > led_finish_time)
            {
                syshal_led_off();
            }
        }

        syshal_sat_tick();
        scheduler_tick();
        syshal_led_tick();

        if (logger_new_data_available)
        {
            logger_push_slots_to_sat();
            logger_new_data_available = false;
        }

        // Go to sleep
        if (!syshal_led_is_active())
        {
            if (syshal_gps_get_state() == SYSHAL_GPS_STATE_ASLEEP)
            {
                uint32_t timestamp_next_alarm = 0;
                scheduler_get_timestamp_next_alarm(&timestamp_next_alarm);

                // We have to kick the hardware watchdog
                if ((timestamp_next_alarm - syshal_rtc_return_timestamp()) > HARD_WATCHDOG_TIMEOUT_S)
                    syshal_rtc_set_alarm(syshal_rtc_return_timestamp() + HARD_WATCHDOG_TIMEOUT_S, NULL);
                else
                    syshal_rtc_set_alarm(timestamp_next_alarm, NULL);

                syshal_pmu_sleep(SLEEP_DEEP);
            }
            else
            {
                uint32_t timestamp_next_alarm = syshal_rtc_return_timestamp() + GPS_ACTIVE_WAKEUP_TIMEOUT_S;
                syshal_rtc_set_alarm(timestamp_next_alarm, NULL);
                syshal_pmu_sleep(SLEEP_DEEP);
            }
        }

        // Branch to Provisioning state if config_if has connected
        if (new_config_available)
            sm_set_next_state(state_handle, SM_MAIN_PROVISIONING);

        // Are we about to leave this state?
        if (sm_is_last_entry(state_handle))
        {
            // Sleep the GPS to save power
            scheduler_gps_stop();
            if (syshal_gps_get_state() != SYSHAL_GPS_STATE_ASLEEP)
                syshal_gps_shutdown();

            if (syshal_sat_get_state() != SYSHAL_SAT_STATE_ASLEEP)
                syshal_sat_shutdown();

            syshal_led_off();

            sensor_logging_enabled = false;
        }
    }
    Catch(e)
    {
        state_message_exception_handler(e);
        sm_set_next_state(state_handle, SM_MAIN_ERROR);
        return;
    }
}

static void sm_main_deactivate(sm_handle_t *state_handle)
{
    CEXCEPTION_T e = CEXCEPTION_NONE;

    Try
    {
        KICK_WATCHDOG();

        uint32_t state_entry_time = 0;

        if (sm_is_first_entry(state_handle))
        {
            DEBUG_PR_INFO("Entered state %s from %s",
                          sm_main_state_str[sm_get_current_state(state_handle)],
                          sm_main_state_str[sm_get_last_state(state_handle)]);

            state_entry_time = syshal_time_get_ticks_ms();
            syshal_led_set_solid(SYSHAL_LED_COLOUR_ORANGE);

            syshal_ble_term();
        }

        if (syshal_time_get_ticks_ms() - state_entry_time >= LED_DEACTIVATED_STATE_DURATION_MS)
            syshal_led_off();

        if (syshal_led_is_active())
            syshal_pmu_sleep(SLEEP_LIGHT);
        else
            syshal_pmu_sleep(SLEEP_DEEP);

        syshal_led_tick();

        if (sm_is_last_entry(state_handle))
        {
            syshal_led_off();
        }
    }
    Catch(e)
    {
        state_message_exception_handler(e);
        sm_set_next_state(state_handle, SM_MAIN_ERROR);
        return;
    }
}

static void sm_main_battery_level_low(sm_handle_t *state_handle)
{
    CEXCEPTION_T e = CEXCEPTION_NONE;

    Try
    {
        KICK_WATCHDOG();

        if (sm_is_first_entry(state_handle))
        {
            DEBUG_PR_INFO("Entered state %s from %s",
                          sm_main_state_str[sm_get_current_state(state_handle)],
                          sm_main_state_str[sm_get_last_state(state_handle)]);
        }

        syshal_pmu_sleep(SLEEP_DEEP);

        if (sm_is_last_entry(state_handle))
        {
            // WILL NEVER ESCAPE THIS MODE UNLESS ...
        }
    }
    Catch(e)
    {
        state_message_exception_handler(e);
        sm_set_next_state(state_handle, SM_MAIN_ERROR);
        return;
    }
}

static void sm_main_log_file_full(sm_handle_t *state_handle)
{
    CEXCEPTION_T e = CEXCEPTION_NONE;

    Try
    {
        KICK_WATCHDOG();

        if (sm_is_first_entry(state_handle))
        {
            DEBUG_PR_INFO("Entered state %s from %s",
                          sm_main_state_str[sm_get_current_state(state_handle)],
                          sm_main_state_str[sm_get_last_state(state_handle)]);
        }

        syshal_pmu_sleep(SLEEP_DEEP);

        if (sm_is_last_entry(state_handle))
        {
            // WILL NEVER ESCAPE THIS MODE UNLESS ...
        }
    }
    Catch(e)
    {
        state_message_exception_handler(e);
        sm_set_next_state(state_handle, SM_MAIN_ERROR);
        return;
    }
}

static void sm_main_error(sm_handle_t *state_handle)
{
    if (sm_is_first_entry(state_handle))
    {
        DEBUG_PR_INFO("Entered state %s from %s",
                      sm_main_state_str[sm_get_current_state(state_handle)],
                      sm_main_state_str[sm_get_last_state(state_handle)]);

        syshal_led_set_blinking(SYSHAL_LED_COLOUR_RED, LED_BLINK_FAIL_DURATION_MS);
    }

    syshal_led_tick();
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// HELPER FUNCTIONS ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static bool check_configuration_tags_set(void)
{
    uint16_t tag, last_index = 0;

#ifndef DEBUG_DISABLED
    static uint16_t last_tag_warned_about = 0xFFFF;
#endif

    while (!sys_config_iterate(&tag, &last_index))
    {
        bool tag_required;
        bool tag_set;

        sys_config_is_required(tag, &tag_required);
        sys_config_is_set(tag, &tag_set);

        if (tag_required && !tag_set)
        {
#ifndef DEBUG_DISABLED
            if (last_tag_warned_about != tag)
            {
                last_tag_warned_about = tag;
                DEBUG_PR_WARN("Configuration tag %d required but not set.", tag);
            }
#endif
            return false;
        }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// BLE_WRITE //////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void ble_write_req(void)
{
    uint8_t buffer[256]; // Magic number
    size_t buffer_size = 0;
    while (ble_stream.available())
    {
        buffer[buffer_size++] = ble_stream.read();
    }
    syshal_ble_send_message(buffer, buffer_size);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// LOGGER //////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void logger_push_slots_to_sat(void)
{
    DEBUG_PR_TRACE("Pushing slot to sat. %s()", __FUNCTION__);

    if (!syshal_sat_wake_up())
    {
        uint32_t prv_slot_epoch = 0xFFFFFFFF;

        for (uint16_t i = 0; i < LOGGER_NB_SLOTS; i++)
        {
            // Get youngest slot from memory
            uint16_t slot_id;
            uint32_t slot_createdDate = 0, slot_acknowledgedDate = 0;
            uint8_t slot_tag;
            uint16_t slot_buffer_size;
            logger_slot_status_id_t status;
            void *slot_buffer;

            logger_get_youngest_slot_id_older_than(&slot_id, &slot_createdDate, prv_slot_epoch);
            prv_slot_epoch = slot_createdDate;

            // Youngest slot id is 0 means no more data in logger
            if (slot_id == 0)
            {
                DEBUG_PR_WARN("All slots are empty.");
                break;
            }

            time_t slot_epoch_t = slot_createdDate;
            DEBUG_PR_TRACE("Found slot with ID: %d, epoch: %s", slot_id, asctime(gmtime(&slot_epoch_t)));

            if (!logger_get_data(slot_id, &slot_buffer, &slot_buffer_size, &slot_tag, &slot_createdDate, &slot_acknowledgedDate, &status))
            {
                if (status == LOGGER_SLOT_STATUS_WAITING_TRANSMIT)
                {
                    // Fill data buffer for terminal
                    uint8_t buffer[ASN_MAX_MSG_SIZE];
                    uint8_t buffer_size = 0;

                    memcpy(&buffer[buffer_size], &slot_tag, sizeof(slot_tag));
                    buffer_size += sizeof(slot_tag);

                    memcpy(&buffer[buffer_size], slot_buffer, slot_buffer_size);
                    buffer_size += slot_buffer_size;

                    if ((slot_tag != LOGGER_TAG_PVT_SLOT) &&
                        (slot_tag != LOGGER_TAG_RAW_SLOT))
                    {
                        memcpy(&buffer[buffer_size], &slot_createdDate, sizeof(slot_createdDate));
                        buffer_size += sizeof(slot_createdDate);
                    }

                    // Push data buffer to terminal untile queue is full
                    if (syshal_sat_send_message(buffer, buffer_size, slot_id))
                        break;
                }
            }
        }

        // Update satellite context
        syshal_sat_read_status(&(sm_context.sat_counters.status));
    }

    syshal_sat_shutdown();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////// MESSAGE STATE EXECUTION CODE ////////////////////////
////////////////////////////////////////////////////////////////////////////////

void state_message_exception_handler(CEXCEPTION_T e)
{
    switch (e)
    {
    case EXCEPTION_BAD_SYS_CONFIG_ERROR_CONDITION:
        DEBUG_PR_ERROR("EXCEPTION_BAD_SYS_CONFIG_ERROR_CONDITION");
        break;

    case EXCEPTION_BLE_ERROR:
        DEBUG_PR_ERROR("EXCEPTION_BLE_SEND_ERROR");
        break;

    case EXCEPTION_GPS_ERROR:
        DEBUG_PR_ERROR("EXCEPTION_GPS_SEND_ERROR");
        break;

    case EXCEPTION_FS_ERROR:
        DEBUG_PR_ERROR("EXCEPTION_FS_ERROR");
        break;

    case EXCEPTION_SAT_ERROR:
        DEBUG_PR_ERROR("EXCEPTION_CELLULAR_SEND_ERROR");
        break;

    case EXCEPTION_BOOT_ERROR:
        DEBUG_PR_ERROR("EXCEPTION_BOOT_ERROR");
        break;

    case EXCEPTION_FLASH_ERROR:
        DEBUG_PR_ERROR("EXCEPTION_FLASH_ERROR");
        break;

    default:
        DEBUG_PR_ERROR("Unknown message exception");
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// STATE HANDLERS ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void sm_main_exception_handler(CEXCEPTION_T e)
{
    switch (e)
    {
    case EXCEPTION_BLE_ERROR:
        DEBUG_PR_ERROR("EXCEPTION_BLE_ERROR");
        break;

    case EXCEPTION_GPS_ERROR:
        DEBUG_PR_ERROR("EXCEPTION_GPS_ERROR");
        break;

    case EXCEPTION_FS_ERROR:
        DEBUG_PR_ERROR("EXCEPTION_FS_ERROR");
        break;

    case EXCEPTION_SPI_ERROR:
        DEBUG_PR_ERROR("EXCEPTION_SPI_ERROR");
        break;
    case EXCEPTION_SAT_ERROR:
        DEBUG_PR_ERROR("EXCEPTION_SAT_ERROR");
        break;

    default:
        DEBUG_PR_ERROR("Unknown state exception %d", e);
        break;
    }
}