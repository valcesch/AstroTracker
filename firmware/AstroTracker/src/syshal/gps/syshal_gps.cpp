/******************************************************************************************
 * File:        syshal_gps.cpp
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

#include "../syshal_gpio.h"
#include "../syshal_gps.h"
#include "../syshal_time.h"
#include "../../core/debug/debug.h"
#include "../syshal_config.h"
#include "SparkFun_u-blox_GNSS_Arduino_Library.h"

SFE_UBLOX_GNSS myGNSS;

static syshal_gps_config_t config;

static syshal_gps_state_t state = SYSHAL_GPS_STATE_UNINIT;
static volatile bool new_data_pending = false;

#define SYSHAL_GPS_GPIO_POWER_ON (GPIO_GPS_EN)
#define SYSHAL_GPS_GPIO_INT (GPIO_GPS_EXT_INT)

#define GPS_NO_FIX 0
#define GPS_DEAD_RECK_ONLY 1
#define GPS_FIX_2D 2
#define GPS_FIX_3D 3
#define GPS_FIX_3D_DEAD_RECK 4
#define GPS_FIX_TIME_ONLY 5

#define SYSHAL_GPS_DEVICE_ADDRESS 0x42

#define SYSHAL_GPS_DELAY_RESTART_MS 400

// Private functions
// ...
/*
static void syshal_gps_int1_pin_interrupt_priv(const syshal_gpio_event_t *event)
{
    new_data_pending = true;
}
*/

int syshal_gps_init(void)
{
    if (state == SYSHAL_GPS_STATE_ASLEEP)
        return SYSHAL_GPS_ERROR_INVALID_STATE;

    // Configure GPIOs
    syshal_gpio_init(SYSHAL_GPS_GPIO_POWER_ON, OUTPUT);
    syshal_gpio_init(SYSHAL_GPS_GPIO_INT, INPUT_PULLDOWN);
    // syshal_gpio_enable_interrupt(SYSHAL_GPS_GPIO_INT, syshal_gps_int1_pin_interrupt_priv);

    // Try establish connection
    syshal_gps_wake_up();
    if (!(myGNSS.begin(Wire, SYSHAL_GPS_DEVICE_ADDRESS)))
    {
        DEBUG_PR_ERROR("Not detected at default I2C address. Please check wiring. %s()", __FUNCTION__);
        syshal_gps_shutdown();
        return SYSHAL_GPS_ERROR_TIMEOUT;
    }

    syshal_gps_shutdown();

    return SYSHAL_GPS_NO_ERROR;
}

int syhsal_gps_update_config(syshal_gps_config_t gps_config)
{
    if (state == SYSHAL_GPS_STATE_ASLEEP)
        return SYSHAL_GPS_ERROR_INVALID_STATE;

    DEBUG_PR_TRACE("Update configuration. %s()", __FUNCTION__);

    config = gps_config;

    if (config.gps->hdr.set)
    {
        // Reset to default configuration
        myGNSS.softwareResetGNSSOnly();

        // Set message output port
        DEBUG_PR_TRACE("Message on I2C port only. %s()", __FUNCTION__);
        myGNSS.setI2COutput(COM_TYPE_UBX); // Set the I2C port to output UBX only (turn off NMEA noise)

        // Set constellations to track
        DEBUG_PR_TRACE("Activate/Deactivate GNSS. %s()", __FUNCTION__);
        myGNSS.enableGNSS(config.gps->contents.with_gps, SFE_UBLOX_GNSS_ID_GPS);
        myGNSS.enableGNSS(config.gps->contents.with_galileo, SFE_UBLOX_GNSS_ID_GALILEO);
        myGNSS.enableGNSS(config.gps->contents.with_beidou, SFE_UBLOX_GNSS_ID_BEIDOU);
        myGNSS.enableGNSS(config.gps->contents.with_glonass, SFE_UBLOX_GNSS_ID_GLONASS);

        // Enable / disable raw messages
        if (config.gps->contents.with_rxm_meas20)
        {
            DEBUG_PR_TRACE("Activate: UBX - UBX_CLASS_RXM - UBX_RXM_MEAS20. %s()", __FUNCTION__);
            myGNSS.enableMessage(UBX_CLASS_RXM, UBX_RXM_MEAS20, COM_PORT_I2C);
        }

        // myGNSS.setNavigationFrequency(10); // Set output to 1 times a second
    }

    return SYSHAL_GPS_NO_ERROR;
}

int syshal_gps_term(void)
{
    myGNSS.end(); // Free memory of GNSS module

    return SYSHAL_GPS_NO_ERROR;
}

int syshal_gps_shutdown(void)
{
    if (state == SYSHAL_GPS_STATE_ASLEEP)
        return SYSHAL_GPS_NO_ERROR; // GPS is already shutdown

    syshal_gpio_set_output_low(SYSHAL_GPS_GPIO_POWER_ON);

    DEBUG_PR_TRACE("Shutdown. %s()", __FUNCTION__);

    state = SYSHAL_GPS_STATE_ASLEEP;

    syshal_gps_event_t event;
    event.id = SYSHAL_GPS_EVENT_POWERED_OFF;
    syshal_gps_callback(&event);

    return SYSHAL_GPS_NO_ERROR;
}

int syshal_gps_wake_up(void)
{
    if ((state == SYSHAL_GPS_STATE_ACQUIRING) || (state == SYSHAL_GPS_STATE_FIXED))
        return SYSHAL_GPS_NO_ERROR; // GPS is already awake

    DEBUG_PR_TRACE("Wakeup. %s()", __FUNCTION__);

    syshal_gpio_set_output_high(SYSHAL_GPS_GPIO_POWER_ON);
    syshal_time_delay_ms(SYSHAL_GPS_DELAY_RESTART_MS);

    state = SYSHAL_GPS_STATE_ACQUIRING;

    syshal_gps_event_t event;
    event.id = SYSHAL_GPS_EVENT_POWERED_ON;
    syshal_gps_callback(&event);

    return SYSHAL_GPS_NO_ERROR;
}

int syshal_gps_tick(void)
{
    if (state == SYSHAL_GPS_STATE_UNINIT)
        return SYSHAL_GPS_ERROR_INVALID_STATE;
    if (state == SYSHAL_GPS_STATE_ASLEEP)
        return SYSHAL_GPS_NO_ERROR; // Ignore messages received after shutdown

    DEBUG_PR_TRACE("Process EVENT... %s()", __FUNCTION__);

    syshal_gps_event_t event;

    // Update variables (needed for getting correct fix time if GPS power cycling)
    myGNSS.getPVT();

    // STATUS event
    /* // Long acquisition time
    if (myGNSS.getNAVSTATUS())
    {
        event.id = SYSHAL_GPS_EVENT_STATUS;

        event.status.iTOW = myGNSS.getTimeOfWeek();
        event.status.gpsFix = myGNSS.getFixType();
        event.status.flags = 0; //  Not implemented
        event.status.fixStat = myGNSS.getGnssFixOk();
        event.status.flags2 = 0; //  Not implemented
        event.status.ttff = 0;   //  Not implemented
        event.status.msss = 0;   //  Not implemented

        DEBUG_PR_TRACE("Got STATUS. %s()", __FUNCTION__);

        syshal_gps_callback(&event);
    }
    */

    uint8_t meas20[20];
    if (myGNSS.getRXMMEAS20(meas20))
    {
        myGNSS.flushRXMMEAS20();
        
        event.id = SYSHAL_GPS_EVENT_RAW;

        memcpy(event.raw.meas20, meas20, 20);
        event.raw.timestamp = syshal_rtc_return_timestamp();

        DEBUG_PR_TRACE("Got RAW message. %s()", __FUNCTION__);

        state = SYSHAL_GPS_STATE_FIXED_RAW; // Before tick()

        syshal_gps_callback(&event);
    }

    // PVT event
    if (myGNSS.getFixType() == GPS_FIX_3D)
    {
        event.id = SYSHAL_GPS_EVENT_PVT;

        event.pvt.timestamp_valid = true; // 3D fix
        event.pvt.iTOW = myGNSS.getTimeOfWeek();
        event.pvt.gpsFix = myGNSS.getFixType();
        event.pvt.lon = myGNSS.getLongitude();
        event.pvt.lat = myGNSS.getLatitude();
        event.pvt.hMSL = myGNSS.getAltitudeMSL();
        event.pvt.hAcc = myGNSS.getHorizontalAccuracy();
        event.pvt.vAcc = myGNSS.getVerticalAccuracy();
        event.pvt.timestamp = myGNSS.getUnixEpoch();
        event.pvt.SIV = myGNSS.getSIV();
        event.pvt.gSpeed = myGNSS.getGroundSpeed();

        DEBUG_PR_TRACE("Got PVT. %s()", __FUNCTION__);

        state = SYSHAL_GPS_STATE_FIXED; // Before tick()

        syshal_gps_callback(&event);
    }

    return SYSHAL_GPS_NO_ERROR;
}

syshal_gps_state_t syshal_gps_get_state(void)
{
    return state;
}

__attribute__((weak)) void syshal_gps_callback(syshal_gps_event_t *event)
{
    DEBUG_PR_WARN("%s Not implemented", __FUNCTION__);
}