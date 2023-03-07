/******************************************************************************************
 * File:        syshal_rtc.h
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

#ifndef _SYSHAL_RTC_H_
#define _SYSHAL_RTC_H_

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#define SYSHAL_RTC_NO_ERROR (0)
#define SYSHAL_RTC_ERROR_DEVICE (-1)
#define SYSHAL_RTC_ERROR_BUSY (-2)
#define SYSHAL_RTC_ERROR_TIMEOUT (-3)
#define SYSHAL_RTC_INVALID_PARAMETER (-4)
#define SYSHAL_RTC_ERROR_SET_WDT (-5)

int syshal_rtc_init(void);
int syshal_rtc_term(void);
int syshal_rtc_set_timestamp(uint32_t timestamp);
uint32_t syshal_rtc_return_timestamp(void);
int syshal_rtc_get_timestamp(uint32_t *timestamp);
int syshal_rtc_get_uptime(uint32_t *uptime);
uint32_t syshal_rtc_return_uptime(void);
// int syshal_rtc_stash_time(void);

int syshal_rtc_soft_watchdog_set(unsigned int seconds);
int syshal_rtc_soft_watchdog_enable(void);
int syshal_rtc_soft_watchdog_disable(void);
int syshal_rtc_soft_watchdog_running(bool *is_running);
int syshal_rtc_soft_watchdog_refresh(void);

int syshal_rtc_set_alarm(const uint32_t timestamp, const voidFuncPtr callback);
int syshal_rtc_disable_alarm(void);

void syshal_rtc_wakeup_event(void);

#endif /* _SYSHAL_RTC_H_ */
