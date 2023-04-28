/******************************************************************************************
 * File:        astronode.cpp
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

#include "astronode.h"
#include "../../core/debug/debug.h"

ans_status_e ASTRONODE::begin(Stream &serialPort)
{
  DEBUG_PR_TRACE("Connecting to module. %s()", __FUNCTION__);

  _serialPort = &serialPort;

  // Set-up UART
  _serialPort->setTimeout(TIMEOUT_SERIAL);

  // Clear buffer
  while (_serialPort->available() > 0)
  {
    _serialPort->read();
  }

  // Send dummy command (known bug in Astronode S)
  dummy_cmd();

  return ANS_STATUS_SUCCESS;
}

void ASTRONODE::end()
{
  // Empty
}

ans_status_e ASTRONODE::configuration_write(bool with_pl_ack,
                                            bool with_geoloc,
                                            bool with_ephemeris,
                                            bool with_deep_sleep,
                                            bool with_ack_event_pin_mask,
                                            bool with_reset_event_pin_mask,
                                            bool with_cmd_event_pin_mask,
                                            bool with_tx_pend_event_pin_mask)
{
  DEBUG_PR_TRACE("Set configuration. %s()", __FUNCTION__);
  DEBUG_PR_TRACE("with_pl_ack = %d", with_pl_ack);
  DEBUG_PR_TRACE("with_geoloc = %d", with_geoloc);
  DEBUG_PR_TRACE("with_ephemeris = %d", with_ephemeris);
  DEBUG_PR_TRACE("with_deep_sleep = %d", with_deep_sleep);
  DEBUG_PR_TRACE("with_ack_event_pin_mask = %d", with_ack_event_pin_mask);
  DEBUG_PR_TRACE("with_reset_event_pin_mask = %d", with_reset_event_pin_mask);
  DEBUG_PR_TRACE("with_cmd_event_pin_mask = %d", with_cmd_event_pin_mask);
  DEBUG_PR_TRACE("with_tx_pend_event_pin_mask = %d", with_tx_pend_event_pin_mask);
  
  // Set parameters
  uint8_t param_w[3] = {};

  if (with_pl_ack)
    param_w[0] |= 1 << 0; // Satellite Acknowledgement
  if (with_geoloc)
    param_w[0] |= 1 << 1; // Add Geolocation
  if (with_ephemeris)
    param_w[0] |= 1 << 2; // Enable Ephemeris
  if (with_deep_sleep)
    param_w[0] |= 1 << 3; // Deep Sleep Mode
  if (with_ack_event_pin_mask)
    param_w[2] |= 1 << 0; // Satellite Ack Event Pin Mask
  if (with_reset_event_pin_mask)
    param_w[2] |= 1 << 1; // Reset Notification Event Pin Mask
  if (with_cmd_event_pin_mask)
    param_w[2] |= 1 << 2; // Command Available Event Pin Mask
  if (with_tx_pend_event_pin_mask)
    param_w[2] |= 1 << 3; // Message Transmission (Tx) Pending Event Pin Mask

  // Send request
  uint8_t reg = CFG_WR;
  ans_status_e ret_val = encode_send_request(reg, param_w, sizeof(param_w));
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, NULL, 0);
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == CFG_WA)
    {
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::configuration_read(ASTRONODE_CONFIG *config)
{
  DEBUG_PR_TRACE("Read configuration. %s()", __FUNCTION__);

  // Set parameters
  uint8_t param_a[8] = {};

  // Send request
  uint8_t reg = CFG_RR;
  ans_status_e ret_val = encode_send_request(reg, NULL, 0);
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, param_a, sizeof(param_a));
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == CFG_RA)
    {
      config->product_id = param_a[0];
      config->hardware_rev = param_a[1];
      config->firmware_maj_ver = param_a[2];
      config->firmware_min_ver = param_a[3];
      config->firmware_rev = param_a[4];
      config->with_pl_ack = (param_a[5] & (1 << 0));
      config->with_geoloc = (param_a[5] & (1 << 1));
      config->with_ephemeris = (param_a[5] & (1 << 2));
      config->with_deep_sleep_en = (param_a[5] & (1 << 3));
      config->with_msg_ack_pin_en = (param_a[7] & (1 << 0));
      config->with_msg_reset_pin_en = (param_a[7] & (1 << 1));

      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::configuration_save(void)
{
  DEBUG_PR_TRACE("Save configuration. %s()", __FUNCTION__);

  // Set parameters
  // None

  // Send request
  uint8_t reg = CFG_SR;
  ans_status_e ret_val = encode_send_request(reg, NULL, 0);

  // delay(TIMEOUT_FLASH); // Not ideal location

  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, NULL, 0);
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == CFG_SA)
    {
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::wifi_configuration_write(const char *wland_ssid,
                                                 const char *wland_key,
                                                 const char *auth_token)
{
  DEBUG_PR_TRACE("Set WiFi configuration. %s()", __FUNCTION__);

  // Set parameters
  uint8_t param_w[194] = {'\0'};

  uint8_t wland_ssid_length = strlen(wland_ssid);
  uint8_t wland_key_length = strlen(wland_key);
  uint8_t auth_token_length = strlen(auth_token);

  memcpy(&param_w[0], wland_ssid, wland_ssid_length);
  memcpy(&param_w[33], wland_key, wland_key_length);
  memcpy(&param_w[97], auth_token, auth_token_length);

  // Send request
  uint8_t reg = WIF_WR;
  ans_status_e ret_val = encode_send_request(reg, param_w, sizeof(param_w));
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, NULL, 0);
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == WIF_WA)
    {
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::satellite_search_config_write(uint8_t search_period,
                                                      bool force_search)
{
  DEBUG_PR_TRACE("Set satellite search rate: rate = %d, force = %d. %s()", search_period, force_search, __FUNCTION__);

  // Set parameters
  uint8_t param_w[2] = {};

  param_w[0] = search_period;

  if (force_search)
    param_w[1] |= 1 << 0;

  // Send request
  uint8_t reg = SSC_WR;
  ans_status_e ret_val = encode_send_request(reg, param_w, sizeof(param_w));
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, NULL, 0);
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == SSC_WA)
    {
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::geolocation_write(int32_t lat,
                                          int32_t lon)
{
  DEBUG_PR_TRACE("Set geolocation lat = %d, lon = %d. %s()", lat, lon, __FUNCTION__);

  // Set parameters
  uint8_t param_w[8] = {};

  memcpy(&param_w[0], &lat, sizeof(lat));
  memcpy(&param_w[4], &lon, sizeof(lon));

  // Send request
  uint8_t reg = GEO_WR;
  ans_status_e ret_val = encode_send_request(reg, param_w, sizeof(param_w));
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, NULL, 0);
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == GEO_WA)
    {
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::factory_reset(void)
{
  DEBUG_PR_TRACE("Factory reset. %s()", __FUNCTION__);

  // Set parameters
  // None

  // Send request
  uint8_t reg = CFG_FR;
  ans_status_e ret_val = encode_send_request(reg, NULL, 0);

  // delay(TIMEOUT_FLASH); // Not ideal location

  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, NULL, 0);
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == CFG_FA)
    {
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::guid_read(String *guid)
{
  DEBUG_PR_TRACE("Read GUID. %s()", __FUNCTION__);

  // Set parameters
  uint8_t param_a[36] = {};

  // Send request
  uint8_t reg = MGI_RR;
  ans_status_e ret_val = encode_send_request(reg, NULL, 0);
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, param_a, sizeof(param_a));
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == MGI_RA)
    {
      for (uint8_t i = 0; i < sizeof(param_a); i++)
      {
        guid->concat((char)param_a[i]);
      }
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::serial_number_read(String *sn)
{
  DEBUG_PR_TRACE("Read SN. %s()", __FUNCTION__);

  // Set parameters
  uint8_t param_a[16] = {};

  // Send request
  uint8_t reg = MSN_RR;
  ans_status_e ret_val = encode_send_request(reg, NULL, 0);
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, param_a, sizeof(param_a));
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == MSN_RA)
    {
      for (uint8_t i = 0; i < sizeof(param_a); i++)
      {
        sn->concat((char)param_a[i]);
      }
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::product_number_read(String *pn)
{
  DEBUG_PR_TRACE("Read PN. %s()", __FUNCTION__);

  // Set parameters
  uint8_t param_a[16] = {};

  // Send request
  uint8_t reg = MPN_RR;
  ans_status_e ret_val = encode_send_request(reg, NULL, 0);
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, param_a, sizeof(param_a));
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == MPN_RA)
    {
      for (uint8_t i = 0; i < sizeof(param_a); i++)
      {
        pn->concat((char)param_a[i]);
      }
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::rtc_read(uint32_t *time)
{
  DEBUG_PR_TRACE("Read RTC. %s()", __FUNCTION__);

  // Set parameters
  uint8_t param_a[4] = {};

  // Send request
  uint8_t reg = RTC_RR;
  ans_status_e ret_val = encode_send_request(reg, NULL, 0);
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, param_a, sizeof(param_a));
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == RTC_RA)
    {
      uint32_t time_tmp = (((uint32_t)param_a[3]) << 24) +
                          (((uint32_t)param_a[2]) << 16) +
                          (((uint32_t)param_a[1]) << 8) +
                          (((uint32_t)param_a[0]) << 0);
      *time = time_tmp + ASTROCAST_REF_UNIX_TIME;
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::read_next_contact_opportunity(uint32_t *delay)
{
  DEBUG_PR_TRACE("Read next contact opportunity. %s()", __FUNCTION__);

  // Set parameters
  uint8_t param_a[4] = {};

  // Send request
  uint8_t reg = NCO_RR;
  ans_status_e ret_val = encode_send_request(reg, NULL, 0);
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, param_a, sizeof(param_a));
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == NCO_RA)
    {
      uint32_t delay_tmp = (((uint32_t)param_a[3]) << 24) +
                           (((uint32_t)param_a[2]) << 16) +
                           (((uint32_t)param_a[1]) << 8) +
                           (((uint32_t)param_a[0] << 0));
      *delay = delay_tmp;
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::read_performance_counter(ASTRONODE_PER_STRUCT *per_struct)
{
  DEBUG_PR_TRACE("Read performance counter. %s()", __FUNCTION__);

  // Set parameters
  uint8_t param_a[PER_CMD_LENGTH] = {};

  // Send request
  uint8_t reg = PER_RR;
  ans_status_e ret_val = encode_send_request(reg, NULL, 0);
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, param_a, sizeof(param_a));
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == PER_RA)
    {
      uint8_t i = 0;
      do
      {
        uint8_t type = param_a[i++];
        uint8_t length = param_a[i++];
        switch (type)
        {
        case PER_TYPE_SAT_SEARCH_PHASE_CNT:
          if (length == sizeof(per_struct->sat_search_phase_cnt))
            memcpy(&per_struct->sat_search_phase_cnt, &param_a[i], length);
          break;
        case PER_TYPE_SAT_DETECT_OPERATION_CNT:
          if (length == sizeof(per_struct->sat_detect_operation_cnt))
            memcpy(&per_struct->sat_detect_operation_cnt, &param_a[i], length);
          break;
        case PER_TYPE_SIGNAL_DEMOD_PHASE_CNT:
          if (length == sizeof(per_struct->signal_demod_phase_cnt))
            memcpy(&per_struct->signal_demod_phase_cnt, &param_a[i], length);
          break;
        case PER_TYPE_SIGNAL_DEMOD_ATTEMPS_CNT:
          if (length == sizeof(per_struct->signal_demod_attempt_cnt))
            memcpy(&per_struct->signal_demod_attempt_cnt, &param_a[i], length);
          break;
        case PER_TYPE_SIGNAL_DEMOD_SUCCESS_CNT:
          if (length == sizeof(per_struct->signal_demod_success_cnt))
            memcpy(&per_struct->signal_demod_success_cnt, &param_a[i], length);
          break;
        case PER_TYPE_ACK_DEMOD_ATTEMPT_CNT:
          if (length == sizeof(per_struct->ack_demod_attempt_cnt))
            memcpy(&per_struct->ack_demod_attempt_cnt, &param_a[i], length);
          break;
        case PER_TYPE_ACK_DEMOD_SUCCESS_CNT:
          if (length == sizeof(per_struct->ack_demod_success_cnt))
            memcpy(&per_struct->ack_demod_success_cnt, &param_a[i], length);
          break;
        case PER_TYPE_QUEUED_MSG_CNT:
          if (length == sizeof(per_struct->queued_msg_cnt))
            memcpy(&per_struct->queued_msg_cnt, &param_a[i], length);
          break;
        case PER_TYPE_DEQUEUED_UNACK_MSG_CNT:
          if (length == sizeof(per_struct->dequeued_unack_msg_cnt))
            memcpy(&per_struct->dequeued_unack_msg_cnt, &param_a[i], length);
          break;
        case PER_TYPE_ACK_MSG_CNT:
          if (length == sizeof(per_struct->ack_msg_cnt))
            memcpy(&per_struct->ack_msg_cnt, &param_a[i], length);
          break;
        case PER_TYPE_SENT_FRAGMENT_CNT:
          if (length == sizeof(per_struct->sent_fragment_cnt))
            memcpy(&per_struct->sent_fragment_cnt, &param_a[i], length);
          break;
        case PER_TYPE_ACK_FRAGMENT_CNT:
          if (length == sizeof(per_struct->ack_fragment_cnt))
            memcpy(&per_struct->ack_fragment_cnt, &param_a[i], length);
          break;
        case PER_TYPE_CMD_DEMOD_ATTEMPT_CNT:
          if (length == sizeof(per_struct->cmd_demod_attempt_cnt))
            memcpy(&per_struct->cmd_demod_attempt_cnt, &param_a[i], length);
          break;
        case PER_TYPE_CMD_DEMOD_SUCCESS_CNT:
          if (length == sizeof(per_struct->cmd_demod_success_cnt))
            memcpy(&per_struct->cmd_demod_success_cnt, &param_a[i], length);
          break;
        }
        i += length;
      } while (i < PER_CMD_LENGTH);
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::save_performance_counter(void)
{
  DEBUG_PR_TRACE("Save performance counter. %s()", __FUNCTION__);

  // Set parameters
  // None

  // Send request
  uint8_t reg = PER_SR;
  ans_status_e ret_val = encode_send_request(reg, NULL, 0);
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, NULL, 0);
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == PER_SA)
    {
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::clear_performance_counter(void)
{
  DEBUG_PR_TRACE("Clear performance counter. %s()", __FUNCTION__);

  // Set parameters
  // None

  // Send request
  uint8_t reg = PER_CR;
  ans_status_e ret_val = encode_send_request(reg, NULL, 0);
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, NULL, 0);
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == PER_CA)
    {
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::read_module_state(ASTRONODE_MST_STRUCT *mst_struct)
{
  DEBUG_PR_TRACE("Read module state. %s()", __FUNCTION__);

  // Set parameters
  uint8_t param_a[MST_CMD_LENGTH] = {};

  // Send request
  uint8_t reg = MST_RR;
  ans_status_e ret_val = encode_send_request(reg, NULL, 0);
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, param_a, sizeof(param_a));
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == MST_RA)
    {
      uint8_t i = 0;
      do
      {
        uint8_t type = param_a[i++];
        uint8_t length = param_a[i++];
        switch (type)
        {
        case MST_TYPE_MSG_IN_QUEUE:
          if (length == sizeof(mst_struct->msg_in_queue))
            memcpy(&mst_struct->msg_in_queue, &param_a[i], length);
          break;
        case MST_TYPE_ACK_MSG_QUEUE:
          if (length == sizeof(mst_struct->ack_msg_in_queue))
            memcpy(&mst_struct->ack_msg_in_queue, &param_a[i], length);
          break;
        case MST_TYPE_LAST_RST:
          if (length == sizeof(mst_struct->last_rst))
            memcpy(&mst_struct->last_rst, &param_a[i], length);
          break;
        case MST_UPTIME:
          if (length == sizeof(mst_struct->uptime))
            memcpy(&mst_struct->uptime, &param_a[i], length);
          break;
        }
        i += length;
      } while (i < MST_CMD_LENGTH);
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::read_environment_details(ASTRONODE_END_STRUCT *end_struct)
{
  DEBUG_PR_TRACE("Read environment details. %s()", __FUNCTION__);

  // Set parameters
  uint8_t param_a[END_CMD_LENGTH] = {};

  // Send request
  uint8_t reg = END_RR;
  ans_status_e ret_val = encode_send_request(reg, NULL, 0);
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, param_a, sizeof(param_a));
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == END_RA)
    {
      uint8_t i = 0;
      do
      {
        uint8_t type = param_a[i++];
        uint8_t length = param_a[i++];
        switch (type)
        {
        case END_TYPE_LAST_MAC_RESULT:
          if (length == sizeof(end_struct->last_mac_result))
            memcpy(&end_struct->last_mac_result, &param_a[i], length);
          break;
        case END_TYPE_LAST_SAT_SEARCH_PEAK_RSSI:
          if (length == sizeof(end_struct->last_sat_search_peak_rssi))
            memcpy(&end_struct->last_sat_search_peak_rssi, &param_a[i], length);
          break;
        case END_TYPE_TIME_SINCE_LAST_SAT_SEARCH:
          if (length == sizeof(end_struct->time_since_last_sat_search))
            memcpy(&end_struct->time_since_last_sat_search, &param_a[i], length);
          break;
        }
        i += length;
      } while (i < END_CMD_LENGTH);
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::read_last_contact_details(ASTRONODE_LCD_STRUCT *lcd_struct)
{
  DEBUG_PR_TRACE("Read last contact details. %s()", __FUNCTION__);

  // Set parameters
  uint8_t param_a[LCD_CMD_LENGTH] = {};

  // Send request
  uint8_t reg = LCD_RR;
  ans_status_e ret_val = encode_send_request(reg, NULL, 0);
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, param_a, sizeof(param_a));
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == LCD_RA)
    {
      uint8_t i = 0;
      do
      {
        uint8_t type = param_a[i++];
        uint8_t length = param_a[i++];
        switch (type)
        {
        case LCD_TYPE_TIME_START_LAST_CONTACT:
          if (length == sizeof(lcd_struct->time_start_last_contact))
            memcpy(&lcd_struct->time_start_last_contact, &param_a[i], length);
          break;
        case LCD_TYPE_TIME_END_LAST_CONTACT:
          if (length == sizeof(lcd_struct->time_end_last_contact))
            memcpy(&lcd_struct->time_end_last_contact, &param_a[i], length);
          break;
        case LCD_TYPE_PEAK_RSSI_LAST_CONTACT:
          if (length == sizeof(lcd_struct->peak_rssi_last_contact))
            memcpy(&lcd_struct->peak_rssi_last_contact, &param_a[i], length);
          break;
        case LCD_TYPE_TIME_PEAK_RSSI_LAST_CONTACT:
          if (length == sizeof(lcd_struct->time_peak_rssi_last_contact))
            memcpy(&lcd_struct->time_peak_rssi_last_contact, &param_a[i], length);
          break;
        }
        i += length;
      } while (i < LCD_CMD_LENGTH);
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::enqueue_payload(uint8_t *data,
                                        uint8_t length,
                                        uint16_t id)
{
  DEBUG_PR_TRACE("Enqueue payload: length = %d [B], ID = %d. %s()", length, id, __FUNCTION__);

  ans_status_e ret_val;
  if (length <= ASN_MAX_MSG_SIZE)
  {
    // Set parameters
    uint8_t param_w[160 + 2] = {};
    uint8_t param_a[2] = {};

    param_w[0] = (uint8_t)id;
    param_w[1] = (uint8_t)(id >> 8);

    memcpy(&param_w[2], data, length);

    // Send request
    uint8_t reg = PLD_ER;
    ret_val = encode_send_request(reg, param_w, length + 2);
    if (ret_val == ANS_STATUS_DATA_SENT)
    {
      ret_val = receive_decode_answer(&reg, param_a, sizeof(param_a));
      if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == PLD_EA)
      {
        // Check that enqueued payload has the correct ID
        uint16_t id_check = (((uint16_t)param_a[1]) << 8) + ((uint16_t)param_a[0]);
        if (id == id_check)
        {
          ret_val = ANS_STATUS_SUCCESS;
        }
        else
        {
          ret_val = ANS_STATUS_PAYLOD_ID_CHECK_FAILED;
        }
      }
    }
  }
  else
  {
    ret_val = ANS_STATUS_PAYLOAD_TOO_LONG;
  }

  return ret_val;
}

ans_status_e ASTRONODE::dequeue_payload(uint16_t *id)
{
  DEBUG_PR_TRACE("Dequeue payload: ID = %d. %s()", id, __FUNCTION__);

  // Set parameters
  uint8_t param_a[2] = {};

  // Send request
  uint8_t reg = PLD_DR;
  ans_status_e ret_val = encode_send_request(reg, NULL, 0);
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, param_a, sizeof(param_a));
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == PLD_DA)
    {
      *id = (((uint16_t)param_a[1]) << 8) + ((uint16_t)param_a[0]);
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::clear_free_payloads(void)
{
  DEBUG_PR_TRACE("Clear all payloads. %s()", __FUNCTION__);

  // Set parameters
  // None

  // Send request
  uint8_t reg = PLD_FR;
  ans_status_e ret_val = encode_send_request(reg, NULL, 0);
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, NULL, 0);
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == PLD_FA)
    {
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::event_read(uint8_t *event_type)
{
  DEBUG_PR_TRACE("Read event. %s()", __FUNCTION__);

  // Set parameters
  uint8_t param_a;

  // Send request
  uint8_t reg = EVT_RR;
  ans_status_e ret_val = encode_send_request(reg, NULL, 0);
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, &param_a, sizeof(param_a));
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == EVT_RA)
    {
      if (param_a & (1 << 0))
      {
        *event_type = EVENT_MSG_ACK;
      }
      else if (param_a & (1 << 1))
      {
        *event_type = EVENT_RESET;
      }
      else if (param_a & (1 << 2))
      {
        *event_type = EVENT_CMD_RECEIVED;
      }
      else if (param_a & (1 << 3))
      {
        *event_type = EVENT_MSG_PENDING;
      }
      else
      {
        *event_type = EVENT_NO_EVENT;
      }
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::read_satellite_ack(uint16_t *id)
{
  DEBUG_PR_TRACE("Read satellite ack. %s()", __FUNCTION__);

  // Set parameters
  uint8_t param_a[2] = {};

  // Send request
  uint8_t reg = SAK_RR;
  ans_status_e ret_val = encode_send_request(SAK_RR, NULL, 0);
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, param_a, sizeof(param_a));
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == SAK_RA)
    {
      *id = (((uint16_t)param_a[1]) << 8) + (uint16_t)(param_a[0]);
      DEBUG_PR_TRACE("Satellite ack ID = %d. %s()", *id, __FUNCTION__);
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::clear_satellite_ack(void)
{
  DEBUG_PR_TRACE("Clear satellite ack event. %s()", __FUNCTION__);

  // Set parameters
  // None

  // Send request
  uint8_t reg = SAK_CR;
  ans_status_e ret_val = encode_send_request(reg, NULL, 0);
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, NULL, 0);
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == SAK_CA)
    {
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::clear_reset_event(void)
{
  DEBUG_PR_TRACE("Clear reset event. %s()", __FUNCTION__);

  // Set parameters
  // None

  // Send request
  uint8_t reg = RES_CR;
  ans_status_e ret_val = encode_send_request(reg, NULL, 0);
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, NULL, 0);
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == RES_CA)
    {
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::read_command_8B(uint8_t data[DATA_CMD_8B_SIZE],
                                        uint32_t *createdDate)
{
  DEBUG_PR_TRACE("Read command. %s()", __FUNCTION__);

  // Set parameters
  uint8_t param_a[12] = {};

  // Send request
  uint8_t reg = CMD_RR;
  ans_status_e ret_val = encode_send_request(reg, NULL, 0);
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, param_a, sizeof(param_a));
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == CMD_RA)
    {
      uint32_t time_tmp = (((uint32_t)param_a[3]) << 24) +
                          (((uint32_t)param_a[2]) << 16) +
                          (((uint32_t)param_a[1]) << 8) +
                          (((uint32_t)param_a[0]) << 0);
      *createdDate = time_tmp + ASTROCAST_REF_UNIX_TIME;
      memcpy(data, &param_a[4], DATA_CMD_8B_SIZE);
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::read_command_40B(uint8_t data[DATA_CMD_40B_SIZE],
                                         uint32_t *createdDate)
{
  DEBUG_PR_TRACE("Read command. %s()", __FUNCTION__);

  // Set parameters
  uint8_t param_a[44] = {};

  // Send request
  uint8_t reg = CMD_RR;
  ans_status_e ret_val = encode_send_request(reg, NULL, 0);
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, param_a, sizeof(param_a));
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == CMD_RA)
    {
      uint32_t time_tmp = (((uint32_t)param_a[3]) << 24) +
                          (((uint32_t)param_a[2]) << 16) +
                          (((uint32_t)param_a[1]) << 8) +
                          (((uint32_t)param_a[0]) << 0);
      *createdDate = time_tmp + ASTROCAST_REF_UNIX_TIME;
      memcpy(data, &param_a[4], DATA_CMD_40B_SIZE);
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

ans_status_e ASTRONODE::clear_command(void)
{
  DEBUG_PR_TRACE("Clear command. %s()", __FUNCTION__);

  // Set parameters
  // None

  // Send request
  uint8_t reg = CMD_CR;
  ans_status_e ret_val = encode_send_request(reg, NULL, 0);
  if (ret_val == ANS_STATUS_DATA_SENT)
  {
    ret_val = receive_decode_answer(&reg, NULL, 0);
    if (ret_val == ANS_STATUS_DATA_RECEIVED && reg == CMD_CA)
    {
      ret_val = ANS_STATUS_SUCCESS;
    }
  }
  return ret_val;
}

void ASTRONODE::dummy_cmd(void)
{
  DEBUG_PR_TRACE("Dummy command (will return error code). %s()", __FUNCTION__);

  uint8_t reg = 0x00;
  encode_send_request(reg, NULL, 0);
  receive_decode_answer(&reg, NULL, 0);
}

ans_status_e ASTRONODE::encode_send_request(uint8_t reg,
                                            uint8_t *param,
                                            uint8_t param_length)
{
  ans_status_e ret_val;

  // Compute CRC
  uint16_t cmd_crc = crc_compute(reg, param, param_length, 0xFFFF);

  // Add escape characters
  uint8_t *com_buf_astronode_hex = (uint8_t *)calloc(STX_L + 2 * (REG_L + param_length + CRC_L) + ETX_L, sizeof(uint8_t));
  if (com_buf_astronode_hex == NULL)
  {
    DEBUG_PR_ERROR("Not enought memory could be allocated in asset. %s()", __FUNCTION__);
    ret_val = ANS_STATUS_HW_ERR;
  }
  else
  {
    uint16_t index_buf_cmd_hex = 0;

    // Add escape characters
    com_buf_astronode_hex[index_buf_cmd_hex++] = STX;

    // Translate to hexadecimal
    byte_array_to_hex_array(&reg, REG_L, &com_buf_astronode_hex[index_buf_cmd_hex]);
    index_buf_cmd_hex += 2 * REG_L;
    byte_array_to_hex_array(param, param_length, &com_buf_astronode_hex[index_buf_cmd_hex]);
    index_buf_cmd_hex += 2 * param_length;
    byte_array_to_hex_array((uint8_t *)&cmd_crc, CRC_L, &com_buf_astronode_hex[index_buf_cmd_hex]);
    index_buf_cmd_hex += 2 * CRC_L;

    // Add escape characters
    com_buf_astronode_hex[index_buf_cmd_hex++] = ETX;

    // Write command
    if (_serialPort->write(com_buf_astronode_hex, index_buf_cmd_hex) == (size_t)(index_buf_cmd_hex))
    {
      ret_val = ANS_STATUS_DATA_SENT;
    }
    else
    {
      ret_val = ANS_STATUS_HW_ERR;
    }

    print_error_code_string(ret_val);

    free(com_buf_astronode_hex);
  }

  return ret_val;
}

ans_status_e ASTRONODE::receive_decode_answer(uint8_t *reg,
                                              uint8_t *param,
                                              uint8_t param_length)
{
  ans_status_e ret_val;

  // Read answer
  uint16_t max_rx_length = STX_L + 2 * (REG_L + param_length + CRC_L) + ETX_L + 64; // +64 for Communication error - Not clean fix
  if (max_rx_length < (STX_L + 2 * (REG_L + PERR_L + CRC_L) + ETX_L))
  {
    max_rx_length = STX_L + 2 * (REG_L + PERR_L + CRC_L) + ETX_L; // Account at least for error code
  }
  uint8_t *com_buf_astronode_hex = (uint8_t *)calloc(max_rx_length, sizeof(uint8_t));
  if (com_buf_astronode_hex == NULL)
  {
    DEBUG_PR_ERROR("Not enought memory could be allocated in asset. %s()", __FUNCTION__);
    ret_val = ANS_STATUS_HW_ERR;
  }
  else
  {
    uint16_t index_buf_cmd_hex = 0;

    size_t rx_length = _serialPort->readBytesUntil(ETX, (char *)com_buf_astronode_hex, max_rx_length);

    if (rx_length >= (STX_L + 2 * (REG_L + CRC_L))) // At least STX (1), REG(2), CRC (4) (ETX ignored by function)
    {
      // Look for start of the frame
      for (uint16_t i = 0; i < max_rx_length; i++)
      {
        if (com_buf_astronode_hex[i] == STX_L)
        {
          index_buf_cmd_hex = i;
          break;
        }
      }

      // Translate to binary
      uint16_t cmd_crc_check = 0xFFFF, cmd_crc = 0xFFFF;
      index_buf_cmd_hex += STX_L;

      // Register extraction
      hex_array_to_byte_array(&com_buf_astronode_hex[index_buf_cmd_hex], 2 * REG_L, reg); // Skip STX, ETX not in buffer
      index_buf_cmd_hex += 2 * REG_L;

      // Parameter extraction (handle error cases)
      if (*reg == ERR_RA)
      {
        uint8_t param_err[PERR_L]; // handle case where param = NULL
        hex_array_to_byte_array(&com_buf_astronode_hex[index_buf_cmd_hex], 2 * PERR_L, param_err);
        index_buf_cmd_hex += 2 * PERR_L;
        cmd_crc = crc_compute(*reg, param_err, PERR_L, 0xFFFF);
        ret_val = (ans_status_e)((((uint16_t)param_err[1]) << 8) + (uint16_t)(param_err[0]));
      }
      else
      {
        hex_array_to_byte_array(&com_buf_astronode_hex[index_buf_cmd_hex], 2 * param_length, param);
        index_buf_cmd_hex += 2 * param_length;
        cmd_crc = crc_compute(*reg, param, param_length, 0xFFFF);
        ret_val = ANS_STATUS_DATA_RECEIVED;
      }

      // CRC extraction
      hex_array_to_byte_array(&com_buf_astronode_hex[index_buf_cmd_hex], 2 * PERR_L, (uint8_t *)&cmd_crc_check);
      index_buf_cmd_hex += 2 * PERR_L;

      // Verify CRC
      if (cmd_crc != cmd_crc_check)
      {
        ret_val = ANS_STATUS_CRC_NOT_VALID;
      }
    }
    else
    {
      ret_val = ANS_STATUS_TIMEOUT;
    }

    print_error_code_string(ret_val);

    free(com_buf_astronode_hex);
    //_serialPort->flush();  // Not implemented in NeoStream
    while (_serialPort->available()) // Consume remaining bytes in the buffer if any
      _serialPort->read();
  }

  return ret_val;
}

void ASTRONODE::print_error_code_string(uint16_t code)
{
  switch (code)
  {
  case ANS_STATUS_CRC_NOT_VALID:
    DEBUG_PR_ERROR("Discrepancy between provided CRC and expected CRC. %s()", __FUNCTION__);
    break;
  case ANS_STATUS_LENGTH_NOT_VALID:
    DEBUG_PR_ERROR("Message exceeds the maximum length for a frame. %s()", __FUNCTION__);
    break;
  case ANS_STATUS_OPCODE_NOT_VALID:
    DEBUG_PR_ERROR("Invalid Operation Code used. %s()", __FUNCTION__);
    break;
  case ANS_STATUS_ARG_NOT_VALID:
    DEBUG_PR_ERROR("Invalid argument used. %s()", __FUNCTION__);
    break;
  case ANS_STATUS_FLASH_WRITING_FAILED:
    DEBUG_PR_ERROR("Failed to write to the flash. %s()", __FUNCTION__);
    break;
  case ANS_STATUS_DEVICE_BUSY:
    DEBUG_PR_ERROR("Device is busy. %s()", __FUNCTION__);
    break;
  case ANS_STATUS_FORMAT_NOT_VALID:
    DEBUG_PR_ERROR("At least one of the fields (SSID, password, token) is not composed of exclusively printable standard ASCII characters (0x20 to 0x7E). %s()", __FUNCTION__);
    break;
  case ANS_STATUS_PERIOD_INVALID:
    DEBUG_PR_ERROR("The Satellite Search Config period enumeration value is not valid. %s()", __FUNCTION__);
    break;
  case ANS_STATUS_BUFFER_FULL:
    DEBUG_PR_ERROR("Failed to queue the payload because the sending queue is already full. %s()", __FUNCTION__);
    break;
  case ANS_STATUS_DUPLICATE_ID:
    DEBUG_PR_ERROR("Failed to queue the payload because the Payload ID provided by the asset is already in use in the terminal queue. %s()", __FUNCTION__);
    break;
  case ANS_STATUS_BUFFER_EMPTY:
    DEBUG_PR_ERROR("Failed to dequeue a payload from the buffer because the buffer is empty. %s()", __FUNCTION__);
    break;
  case ANS_STATUS_INVALID_POS:
    DEBUG_PR_ERROR("Invalid position. %s()", __FUNCTION__);
    break;
  case ANS_STATUS_NO_ACK:
    DEBUG_PR_ERROR("No satellite acknowledgement available for any payload. %s()", __FUNCTION__);
    break;
  case ANS_STATUS_NO_ACK_CLEAR:
    DEBUG_PR_ERROR("No payload ack to clear, or it was already cleared. %s()", __FUNCTION__);
    break;
  case ANS_STATUS_NO_COMMAND:
    DEBUG_PR_ERROR("No command is available. %s()", __FUNCTION__);
    break;
  case ANS_STATUS_NO_COMMAND_CLEAR:
    DEBUG_PR_ERROR("No command to clear, or it was already cleared. %s()", __FUNCTION__);
    break;
  case ANS_STATUS_MAX_TX_REACHED:
    DEBUG_PR_ERROR("Failed to test Tx due to the maximum number of transmissions being reached. %s()", __FUNCTION__);
    break;
  case ANS_STATUS_TIMEOUT:
    DEBUG_PR_ERROR("Failed to receive data from astronode before timeout. %s()", __FUNCTION__);
    break;
  case ANS_STATUS_HW_ERR:
    DEBUG_PR_ERROR("Failed to send data to the terminal. %s()", __FUNCTION__);
    break;
  case ANS_STATUS_SUCCESS:
    break;
  case ANS_STATUS_DATA_SENT:
    break;
  case ANS_STATUS_DATA_RECEIVED:
    break;
  default:
    DEBUG_PR_ERROR("Unknown error code. %s()", __FUNCTION__);
    break;
  }
}

uint16_t ASTRONODE::crc_compute(uint8_t reg,
                                uint8_t *param,
                                uint16_t param_length,
                                uint16_t init)
{
  uint16_t x;
  uint16_t crc = init;

  x = crc >> 8 ^ reg;
  x ^= x >> 4;
  crc = (crc << 8) ^ (x << 12) ^ (x << 5) ^ (x);

  for (uint8_t i = 0; i < param_length; i++)
  {
    x = crc >> 8 ^ *param++;
    x ^= x >> 4;
    crc = (crc << 8) ^ (x << 12) ^ (x << 5) ^ (x);
  }
  return crc;
}

void ASTRONODE::byte_array_to_hex_array(uint8_t *in,
                                        uint8_t length,
                                        uint8_t *out)
{
  for (int i = 0; i < length; i++)
  {
    uint8_t nibble_h = (in[i] & 0xF0) >> 4;
    uint8_t nibble_l = in[i] & 0x0F;

    out[i << 1] = nibble_to_hex(nibble_h);
    out[(i << 1) + 1] = nibble_to_hex(nibble_l);
  }
}

void ASTRONODE::hex_array_to_byte_array(uint8_t *in,
                                        uint8_t length,
                                        uint8_t *out)
{
  for (int i = 0; i < length; i += 2)
  {
    uint8_t nibble_h = hex_to_nibble(in[i]);
    uint8_t nibble_l = hex_to_nibble(in[i + 1]);

    out[i >> 1] = (nibble_h << 4) + nibble_l;
  }
}

uint8_t ASTRONODE::nibble_to_hex(uint8_t nibble)
{
  if (nibble < 10)
  {
    return nibble + 0x30;
  }
  else
  {
    return (nibble % 0x0A) + 0x41;
  }
}

uint8_t ASTRONODE::hex_to_nibble(uint8_t hex)
{
  if (hex < 0x41)
  {
    return hex - 0x30;
  }
  else
  {
    return (hex - 0x41) + 0x0A; //-0x37
  }
}
