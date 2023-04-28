/******************************************************************************************
 * File:        syshal_ble.cpp
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

#include "../syshal_ble.h"
#include "../syshal_rtc.h"
#include "../../core/debug/debug.h"
#include "../syshal_config.h"

#if defined(NRF52_SERIES) && defined(WITH_BLE)
#include <bluefruit.h>
BLEDis bledis;   // device information
BLEUart bleuart; // uart over ble
#endif

static syshal_ble_config_t config;

#define SYSHAL_BLE_NAME "AstroTracker"
#define SYSHAL_BLE_MANUFACTURER "Astrocast SA"

// Private functions
void syshal_ble_startAdv_priv(void);
void syshal_ble_connect_callback_priv(uint16_t conn_handle);
void syshal_ble_disconnect_callback_priv(uint16_t conn_handle, uint8_t reason);
void syshal_ble_rx_callback_priv(uint16_t conn_handle);

int syshal_ble_init(void)
{
#if defined(NRF52_SERIES) && defined(WITH_BLE)
    // Initialize bletooth
    Bluefruit.autoConnLed(false);
    Bluefruit.configPrphBandwidth(BANDWIDTH_NORMAL);
    Bluefruit.begin();
    Bluefruit.setName(SYSHAL_BLE_NAME);
    Bluefruit.Periph.setConnectCallback(syshal_ble_connect_callback_priv);
    Bluefruit.Periph.setDisconnectCallback(syshal_ble_disconnect_callback_priv);

    // Initialize bluetooth services
    bledis.setManufacturer(SYSHAL_BLE_MANUFACTURER);
    bledis.begin();

    bleuart.begin();
    bleuart.setRxCallback(syshal_ble_rx_callback_priv);
#else
    DEBUG_PR_ERROR("No supported radio. %s()", __FUNCTION__);
#endif
    // Start advertising
    syshal_ble_startAdv_priv();

    return SYSHAL_BLE_NO_ERROR;
}

int syshal_ble_update_config(syshal_ble_config_t ble_config)
{
    DEBUG_PR_TRACE("Update configuration. %s()", __FUNCTION__);

    config = ble_config;

    if (config.ble->hdr.set)
    {
#if defined(NRF52_SERIES) && defined(WITH_BLE)
        DEBUG_PR_TRACE("Set Tx power. %s()", __FUNCTION__);
        Bluefruit.setTxPower(config.ble->contents.tx_power);

        DEBUG_PR_TRACE("Set advertising configuration. %s()", __FUNCTION__);
        /* Start Advertising
         * - Enable auto advertising if disconnected
         * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
         * - Timeout for fast mode is 30 seconds
         * - Start(timeout) with timeout = 0 will advertise forever (until connected)
         *
         * For recommended advertising interval
         * https://developer.apple.com/library/content/qa/qa1931/_index.html
         */
        Bluefruit.Advertising.restartOnDisconnect(true);
        Bluefruit.Advertising.setInterval(config.ble->contents.advert_fast_interval,
                                          config.ble->contents.advert_slow_interval);   // in unit of 0.625 ms
        Bluefruit.Advertising.setFastTimeout(config.ble->contents.advert_fast_timeout); // number of seconds in fast mode
        Bluefruit.Advertising.start(0);                                                 // 0 = Don't stop advertising after n seconds
#else
        DEBUG_PR_ERROR("No supported radio. %s()", __FUNCTION__);
#endif
    }

    return SYSHAL_BLE_NO_ERROR;
}

int syshal_ble_term(void)
{
    DEBUG_PR_WARN("%s Not implemented", __FUNCTION__);

    return SYSHAL_BLE_NO_ERROR;
}

int syshal_ble_send_message(uint8_t *buffer, size_t buffer_size)
{
#if defined(NRF52_SERIES) && defined(WITH_BLE)
    // BLE packets cannot be more than 20 bytes, need to be splitted
    uint8_t buf_size = buffer_size;
    uint8_t buf_idx = 0;
    while (buf_size > 0)
    {
        if (buf_size < 20)
        {
            if (bleuart.write(&buffer[buf_idx], buf_size) != buf_size)
                return SYSHAL_BLE_ERROR_LENGTH;
            buf_size = 0;
        }
        else
        {
            if (bleuart.write(&buffer[buf_idx], 20) != 20)
                return SYSHAL_BLE_ERROR_LENGTH;
            buf_size -= 20;
            buf_idx += 20;
        }
    }
#else
    DEBUG_PR_ERROR("No supported radio. %s()", __FUNCTION__);
#endif

    return SYSHAL_BLE_NO_ERROR;
}

void syshal_ble_rx_callback_priv(uint16_t conn_handle)
{
    (void)conn_handle;

    syshal_ble_event_t event;

    event.cmd_received.buffer_size = 0;

#if defined(NRF52_SERIES) && defined(WITH_BLE)
    while (bleuart.available())
    {
        event.cmd_received.buffer[event.cmd_received.buffer_size++] = bleuart.read();
    }
#else
    DEBUG_PR_ERROR("No supported radio. %s()", __FUNCTION__);
#endif

    event.id = SYSHAL_BLE_EVENT_COMMAND_RECEIVED;

    syshal_ble_callback(&event);
}

void syshal_ble_startAdv_priv(void)
{
    DEBUG_PR_TRACE("Start advertising. %s()", __FUNCTION__);

#if defined(NRF52_SERIES) && defined(WITH_BLE)
    // Advertising packet
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();

    // Include bleuart 128-bit uuid
    Bluefruit.Advertising.addService(bleuart);

    // Secondary Scan Response packet (optional)
    // Since there is no room for 'Name' in Advertising packet
    Bluefruit.ScanResponse.addName();
#else
    DEBUG_PR_ERROR("No supported radio. %s()", __FUNCTION__);
#endif

    syshal_ble_event_t event;
    event.id = SYSHAL_BLE_EVENT_START_ADVERTISING;
    syshal_ble_callback(&event);
}

void syshal_ble_connect_callback_priv(uint16_t conn_handle)
{
    DEBUG_PR_TRACE("Connecting... %s()", __FUNCTION__);

#if defined(NRF52_SERIES) && defined(WITH_BLE)
    // Get the reference to current connection
    BLEConnection *connection = Bluefruit.Connection(conn_handle);

    char central_name[32] = {0};
    connection->getPeerName(central_name, sizeof(central_name));

    DEBUG_PR_TRACE("Connected to %s. %s()", central_name, __FUNCTION__);
#else
    DEBUG_PR_ERROR("No supported radio. %s()", __FUNCTION__);
#endif

    syshal_ble_event_t event;
    event.id = SYSHAL_BLE_EVENT_CONNECTED;
    syshal_ble_callback(&event);
}

void syshal_ble_disconnect_callback_priv(uint16_t conn_handle, uint8_t reason)
{
    (void)conn_handle;
    (void)reason;

    DEBUG_PR_TRACE("Disconnected, reason = 0x%x. %s()", reason, __FUNCTION__);

#if defined(NRF52_SERIES) && defined(WITH_BLE)
    // Empty
#else
    DEBUG_PR_ERROR("No supported radio. %s()", __FUNCTION__);
#endif

    syshal_ble_event_t event;
    event.id = SYSHAL_BLE_EVENT_DISCONNECTED;
    syshal_ble_callback(&event);
}

__attribute__((weak)) void syshal_ble_callback(syshal_ble_event_t *event)
{
    DEBUG_PR_WARN("%s Not implemented", __FUNCTION__);
}