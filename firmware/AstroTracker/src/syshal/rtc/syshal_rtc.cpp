/******************************************************************************************
 * File:        syshal_rtc.cpp
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

#include "../syshal_rtc.h"
#include "../../core/debug/debug.h"
#include "../syshal_config.h"

// All watchdog functions are implemented based on https://github.com/adafruit/Adafruit_SleepyDog

#if defined(NRF52_SERIES)
#include <nrfx_rtc.h>
#include <nrf_soc.h>
#include "nrf_wdt.h"
#define NRF_RTCZ NRF_RTC2
#define NRF_RTC_OVERFLOW_SEC 0x1FFFFF
#endif

#define RTC_TIME_KEEPING_FREQUENCY_HZ (8) // 8 Hz, this is the lowest speed available

static bool soft_watchdog_init;

// uint32_t retained_ram_rtc_timestamp __attribute__((section(".noinit")));

static volatile uint32_t _g_RTC_interrupt_interval = 0; // for periodic interrupts
static volatile voidFuncPtr _g_RTC_callBack = NULL;     // RTC interrupt user handler
static volatile bool _g_f_playing_possum = false;       // flag set by ISR to release spin loop
bool _initialized = false;

static uint32_t timestamp_offset = 0;

void (*functionPointer)(void);

#if defined(NRF52_SERIES)
void RTC2_IRQHandler(void)
{
    nrf_rtc_event_clear(NRF_RTCZ, NRF_RTC_EVENT_COMPARE_0);

    resumeLoop();

    if (functionPointer)
        functionPointer();
}
#endif

#if defined(ARDUINO_ARCH_SAMD)
void RTC_Handler(void)
{
    RTC->MODE0.INTFLAG.reg = RTC_MODE0_INTFLAG_MASK; // clear all interrupt sources

    if (_g_RTC_interrupt_interval != 0)
    {
        RTC->MODE0.COMP[0].reg = RTC->MODE0.COMP[0].reg + _g_RTC_interrupt_interval;
    }

    if (_g_RTC_interrupt_interval == 0)
    {
        syshal_rtc_disable_alarm();
    }

    /*
     * Interrupts cannot be enabled without calling a function that sets
     * RTC_callback so there will never be a stale callback if interrupts
     * are enabled.
     *
     * Putting the callback at the end of the handler allows the callback
     * to set a new or different interrupt.
     */
    if (_g_RTC_callBack != NULL)
    {
        _g_RTC_callBack();
    }

    _g_f_playing_possum = false; // release fake sleep from spin loop
}

void _initialize_wdt()
{
    // One-time initialization of watchdog timer.
    // Insights from rickrlh and rbrucemtl in Arduino forum!

    // Generic clock generator 2, divisor = 32 (2^(DIV+1))
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(2) | GCLK_GENDIV_DIV(4);
    // Enable clock generator 2 using low-power 32KHz oscillator.
    // With /32 divisor above, this yields 1024Hz(ish) clock.
    GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(2) | GCLK_GENCTRL_GENEN |
                        GCLK_GENCTRL_SRC_OSCULP32K | GCLK_GENCTRL_DIVSEL;
    while (GCLK->STATUS.bit.SYNCBUSY)
        ;
    // WDT clock = clock gen 2
    GCLK->CLKCTRL.reg =
        GCLK_CLKCTRL_ID_WDT | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK2;

    // Enable WDT early-warning interrupt
    NVIC_DisableIRQ(WDT_IRQn);
    NVIC_ClearPendingIRQ(WDT_IRQn);
    NVIC_SetPriority(WDT_IRQn, 0); // Top priority
    NVIC_EnableIRQ(WDT_IRQn);

    _initialized = true;
}
#endif

int syshal_rtc_init(void)
{
    timestamp_offset = 0;

#if defined(NRF52_SERIES)
    nrf_rtc_int_enable(NRF_RTCZ, NRF_RTC_INT_OVERFLOW_MASK);
    nrf_rtc_event_enable(NRF_RTCZ, NRF_RTC_EVENT_OVERFLOW);
    nrf_rtc_prescaler_set(NRF_RTCZ, RTC_FREQ_TO_PRESCALER(RTC_TIME_KEEPING_FREQUENCY_HZ));

    NVIC_SetPriority(RTC2_IRQn, 2); // high priority
    NVIC_ClearPendingIRQ(RTC2_IRQn);
    NVIC_EnableIRQ(RTC2_IRQn);

    nrf_rtc_event_clear(NRF_RTCZ, NRF_RTC_EVENT_COMPARE_0);
    nrf_rtc_int_enable(NRF_RTCZ, NRF_RTC_INT_COMPARE0_MASK);

    nrf_rtc_task_trigger(NRF_RTCZ, NRF_RTC_TASK_START);

#elif defined(ARDUINO_ARCH_SAMD)
    // keep the XOSC32K running in standy
    SYSCTRL->XOSC32K.reg |= SYSCTRL_XOSC32K_RUNSTDBY;

    // attach GCLK_RTC to generic clock generator 1
    GCLK->CLKCTRL.reg = (uint32_t)((GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK1 | (RTC_GCLK_ID << GCLK_CLKCTRL_ID_Pos)));

    // ensure module is reset
    RTC->MODE0.CTRL.bit.SWRST = 1;
    while (RTC->MODE0.CTRL.bit.SWRST == 1)
        ;

    // reset configuration is mode=0, no clear on match
    RTC->MODE0.CTRL.reg = RTC_MODE0_CTRL_PRESCALER_DIV1024 | RTC_MODE0_CTRL_ENABLE;

    NVIC_EnableIRQ(RTC_IRQn);
    NVIC_SetPriority(RTC_IRQn, 0x00);

    // reset to zero in case of warm start
    RTC->MODE0.COUNT.reg = 0;
#endif

    return SYSHAL_RTC_NO_ERROR;
}

int syshal_rtc_term(void)
{
    DEBUG_PR_TRACE("%s NOT IMPLEMENTED", __FUNCTION__);

    return SYSHAL_RTC_NO_ERROR;
}

/*
int syshal_rtc_stash_time(void)
{
    // Store our current RTC time into a retained RAM section
    // so that it is maintained through a reset
    uint32_t timestamp;
    int ret = syshal_rtc_get_timestamp(&timestamp);
    if (SYSHAL_RTC_NO_ERROR == ret)
        retained_ram_rtc_timestamp = timestamp;

    return ret;
}
*/

int syshal_rtc_set_timestamp(uint32_t timestamp)
{
    uint32_t uptime;
    syshal_rtc_get_uptime(&uptime);
    timestamp_offset = timestamp - uptime;

    return SYSHAL_RTC_NO_ERROR;
}

uint32_t syshal_rtc_return_timestamp(void)
{
    uint32_t uptime;
    syshal_rtc_get_uptime(&uptime);
    return uptime + timestamp_offset;
}

int syshal_rtc_get_timestamp(uint32_t *timestamp)
{
    uint32_t uptime;
    syshal_rtc_get_uptime(&uptime);
    *timestamp = uptime + timestamp_offset;

    return SYSHAL_RTC_NO_ERROR;
}

int syshal_rtc_get_uptime(uint32_t *uptime)
{
#if defined(NRF52_SERIES)
    *uptime = nrf_rtc_counter_get(NRF_RTCZ) >> 0x3;
    if (nrf_rtc_event_check(NRF_RTCZ, NRF_RTC_EVENT_OVERFLOW))
    {
        nrf_rtc_event_clear(NRF_RTCZ, NRF_RTC_EVENT_OVERFLOW);
        *uptime += NRF_RTC_OVERFLOW_SEC;
    }
#elif defined(ARDUINO_ARCH_SAMD)
    *uptime = ((RTC->MODE0.COUNT.reg) >> 5);
    // TODO: Implement check for overflow
#endif

    return SYSHAL_RTC_NO_ERROR;
}

uint32_t syshal_rtc_return_uptime(void)
{
    uint32_t uptime;
    syshal_rtc_get_uptime(&uptime);
    return uptime;
}

int syshal_rtc_set_alarm(uint32_t timestamp, const voidFuncPtr callback)
{
    uint32_t timeout = timestamp - syshal_rtc_return_timestamp();
    uint32_t uptime;
    syshal_rtc_get_uptime(&uptime);
    uint32_t delay_s = timeout + uptime;

#if defined(NRF52_SERIES)
    functionPointer = callback;
    nrf_rtc_cc_set(NRF_RTCZ, 0, (nrf_rtc_counter_get(NRF_RTCZ) >> 0x3) + delay_s * RTC_TIME_KEEPING_FREQUENCY_HZ);
#elif defined(ARDUINO_ARCH_SAMD)
    syshal_rtc_disable_alarm();
    _g_RTC_callBack = callback;

    // clear any pending interrupts, set compare register and enable interrupt
    RTC->MODE0.INTFLAG.reg = RTC_MODE0_INTFLAG_MASK;
    RTC->MODE0.COMP[0].reg = delay_s << 5;
    RTC->MODE0.INTENSET.bit.CMP0 = 1;
#endif

    return SYSHAL_RTC_NO_ERROR;
}

int syshal_rtc_disable_alarm(void)
{
#if defined(NRF52_SERIES)
    DEBUG_PR_TRACE("%s NOT IMPLEMENTED", __FUNCTION__);
#elif defined(ARDUINO_ARCH_SAMD)
    _g_RTC_interrupt_interval = 0;
    RTC->MODE0.INTENCLR.reg = RTC_MODE0_INTENCLR_MASK;
#endif

    return SYSHAL_RTC_NO_ERROR;
}

int syshal_rtc_soft_watchdog_set(unsigned int seconds)
{
#if defined(NRF52_SERIES)
    // cannot change wdt config register once it is started
    // return previous configured timeout
    if (nrf_wdt_started(NRF_WDT))
        return SYSHAL_RTC_ERROR_SET_WDT;

    // WDT run when CPU is sleep
    nrf_wdt_behaviour_set(NRF_WDT, NRF_WDT_BEHAVIOUR_PAUSE_SLEEP_HALT);
    nrf_wdt_reload_value_set(NRF_WDT, seconds * 32768);

    // use channel 0
    nrf_wdt_reload_request_enable(NRF_WDT, NRF_WDT_RR0);

    // Start WDT
    // After started CRV, RREN and CONFIG is blocked
    // There is no way to stop/disable watchdog using source code
    // It can only be reset by WDT timeout, Pin reset, Power reset
    nrf_wdt_task_trigger(NRF_WDT, NRF_WDT_TASK_START);

#elif defined(ARDUINO_ARCH_SAMD)
    int cycles;
    int maxPeriodMS = seconds * 1000;
    uint8_t bits;

    if (!_initialized)
        _initialize_wdt();

    WDT->CTRL.reg = 0; // Disable watchdog for config
    while (WDT->STATUS.bit.SYNCBUSY)
        ;

    // You'll see some occasional conversion here compensating between
    // milliseconds (1000 Hz) and WDT clock cycles (~1024 Hz).  The low-
    // power oscillator used by the WDT ostensibly runs at 32,768 Hz with
    // a 1:32 prescale, thus 1024 Hz, though probably not super precise.

    if ((maxPeriodMS >= 16000) || !maxPeriodMS)
    {
        cycles = 16384;
        bits = 0xB;
    }
    else
    {
        cycles = (maxPeriodMS * 1024L + 500) / 1000; // ms -> WDT cycles
        if (cycles >= 8192)
        {
            cycles = 8192;
            bits = 0xA;
        }
        else if (cycles >= 4096)
        {
            cycles = 4096;
            bits = 0x9;
        }
        else if (cycles >= 2048)
        {
            cycles = 2048;
            bits = 0x8;
        }
        else if (cycles >= 1024)
        {
            cycles = 1024;
            bits = 0x7;
        }
        else if (cycles >= 512)
        {
            cycles = 512;
            bits = 0x6;
        }
        else if (cycles >= 256)
        {
            cycles = 256;
            bits = 0x5;
        }
        else if (cycles >= 128)
        {
            cycles = 128;
            bits = 0x4;
        }
        else if (cycles >= 64)
        {
            cycles = 64;
            bits = 0x3;
        }
        else if (cycles >= 32)
        {
            cycles = 32;
            bits = 0x2;
        }
        else if (cycles >= 16)
        {
            cycles = 16;
            bits = 0x1;
        }
        else
        {
            cycles = 8;
            bits = 0x0;
        }
    }

    WDT->INTENCLR.bit.EW = 1;   // Disable early warning interrupt
    WDT->CONFIG.bit.PER = bits; // Set period for chip reset
    WDT->CTRL.bit.WEN = 0;      // Disable window mode
    while (WDT->STATUS.bit.SYNCBUSY)
        ; // Sync CTRL write

    syshal_rtc_soft_watchdog_refresh();                  // Clear watchdog interval
    WDT->CTRL.bit.ENABLE = 1; // Start watchdog now!
    while (WDT->STATUS.bit.SYNCBUSY)
        ;
#endif

    soft_watchdog_init = true;

    return SYSHAL_RTC_NO_ERROR;
}

int syshal_rtc_soft_watchdog_enable(void)
{
    syshal_rtc_soft_watchdog_refresh();

#if defined(NRF52_SERIES)
    // DEBUG_PR_TRACE("%s NOT IMPLEMENTED", __FUNCTION__);
#elif defined(ARDUINO_ARCH_SAMD)
    WDT->CTRL.bit.ENABLE = 1; // Start watchdog now!
    while (WDT->STATUS.bit.SYNCBUSY)
        ;
#endif

    soft_watchdog_init = true;

    return SYSHAL_RTC_NO_ERROR;
}

int syshal_rtc_soft_watchdog_disable(void)
{
#if defined(NRF52_SERIES)
    // DEBUG_PR_TRACE("%s NOT IMPLEMENTED", __FUNCTION__);
#elif defined(ARDUINO_ARCH_SAMD)
    WDT->CTRL.bit.ENABLE = 0;
    while (WDT->STATUS.bit.SYNCBUSY)
        ;
#endif

    soft_watchdog_init = false;

    return SYSHAL_RTC_NO_ERROR;
}

int syshal_rtc_soft_watchdog_running(bool *is_running)
{
    *is_running = soft_watchdog_init;

    return SYSHAL_RTC_NO_ERROR;
}

int syshal_rtc_soft_watchdog_refresh(void)
{
    if (!soft_watchdog_init)
        return SYSHAL_RTC_ERROR_DEVICE;

#if defined(NRF52_SERIES)
    nrf_wdt_reload_request_set(NRF_WDT, NRF_WDT_RR0);
#elif defined(ARDUINO_ARCH_SAMD)
    // Write the watchdog clear key value (0xA5) to the watchdog
    // clear register to clear the watchdog timer and reset it.
    while (WDT->STATUS.bit.SYNCBUSY)
        ;
    WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
#endif

    return SYSHAL_RTC_NO_ERROR;
}

__attribute__((weak)) void syshal_rtc_wakeup_event(void)
{
    // NOTE: This should be overriden by the user if it is desired
}