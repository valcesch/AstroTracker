/******************************************************************************************
 * File:        syshal_pmu.cpp
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

#include "../syshal_pmu.h"
#include "../syshal_rtc.h"
#include "../syshal_gpio.h"
#include "../syshal_time.h"
#include "../syshal_sat.h"
#include "../../core/debug/debug.h"
#include "../syshal_config.h"
#include <Wire.h>

#ifdef GPIO_HWDT_RESET
#define SYSHAL_PMU_GPIO_HWDT_RESET (GPIO_HWDT_RESET)
#endif

#define SYSHAL_PMU_HWDT_KICK_HTIME 1 //[us]

static uint32_t reset_reason;

/*
void syshal_pmu_assert_callback(uint16_t line_num, const uint8_t *file_name)
{
    syshal_rtc_stash_time();
#ifdef DONT_RESTART_ASSERT
    DEBUG_PR_ERROR("Assertion %s:%u", file_name, line_num);
    while (1)
    {
    }
#else
    NVIC_SystemReset();
#endif
}
*/

static volatile uint32_t _g_RTC_interrupt_interval = 0; // for periodic interrupts
static volatile voidFuncPtr _g_RTC_callBack = NULL;     // RTC interrupt user handler
static volatile bool _g_f_playing_possum = false;       // flag set by ISR to release spin loop

void syshal_pmu_init(void)
{
    syshal_gpio_init(SYSHAL_PMU_GPIO_HWDT_RESET, OUTPUT);

#if defined(NRF52_SERIES)
    // Empty
#elif defined(ARDUINO_ARCH_SAMD)
    // Empty
#endif
}

void syshal_pmu_sleep(syshal_pmu_sleep_mode_t mode)
{
    bool soft_wdt_running;
    int ret;

// Kick hardware watchdog
#ifdef SYSHAL_PMU_GPIO_HWDT_RESET
    syshal_gpio_set_output_high(SYSHAL_PMU_GPIO_HWDT_RESET);
    syshal_time_delay_us(SYSHAL_PMU_HWDT_KICK_HTIME);
    syshal_gpio_set_output_low(SYSHAL_PMU_GPIO_HWDT_RESET);
#endif

    switch (mode)
    {
    case SLEEP_DEEP:
    {
        // We don't want our soft watchdog to run in deep sleep so disable
        ret = syshal_rtc_soft_watchdog_running(&soft_wdt_running);
        if (ret)
            soft_wdt_running = false;

        if (soft_wdt_running)
        {
            syshal_rtc_soft_watchdog_disable();
        }

        // Go to sleep
#if defined(NRF52_SERIES)
        Wire.end();
        UART_ANS.end();

        nrf_gpio_cfg_default(g_ADigitalPinMap[PIN_WIRE_SDA]);
        nrf_gpio_cfg_default(g_ADigitalPinMap[PIN_WIRE_SCL]);

        nrf_gpio_cfg_default(g_ADigitalPinMap[GPIO_ANS_EXT_INT]);


        suspendLoop();

        UART_ANS.begin(SYSHAL_SAT_BAUDRATE);
        Wire.begin();

#elif defined(ARDUINO_ARCH_SAMD)

        // Disable USB peripheral
        USBDevice.detach();

        // Deactivate SPI peripheral
        PORT->Group[g_APinDescription[PIN_SPI_SCK].ulPort].PINCFG[g_APinDescription[PIN_SPI_SCK].ulPin].reg = 0;
        PORT->Group[g_APinDescription[PIN_SPI_MISO].ulPort].PINCFG[g_APinDescription[PIN_SPI_MISO].ulPin].reg = 0;
        PORT->Group[g_APinDescription[PIN_SPI_MOSI].ulPort].PINCFG[g_APinDescription[PIN_SPI_MOSI].ulPin].reg = 0;
        PORT->Group[g_APinDescription[GPIO_VBAT].ulPort].PINCFG[g_APinDescription[GPIO_VBAT].ulPin].reg = 0;

        // Don't fully power down flash when in sleep
        // NVMCTRL->CTRLB.bit.SLEEPPRM = NVMCTRL_CTRLB_SLEEPPRM_DISABLED_Val;

        // Disable systick interrupt:  See https://www.avrfreaks.net/forum/samd21-samd21e16b-sporadically-locks-and-does-not-wake-standby-sleep-mode
        // Due to a hardware bug on the SAMD21, the SysTick interrupts become
        // active before the flash has powered up from sleep, causing a hard fault.
        // To prevent this the SysTick interrupts are disabled before entering sleep
        // mode.
        SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
        __DSB();
        __WFI();

        // Enable systick interrupt
        SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;

        // Enable back USB peripheral
        USBDevice.attach();
#endif

        // Enable back software watchdog
        syshal_rtc_soft_watchdog_enable();
        break;
    }
    case SLEEP_LIGHT:
    {
        DEBUG_PR_TRACE("%s NOT IMPLEMENTED", __FUNCTION__);
        delay(5000);
        break;
    }
    default:
        break;
    }
}

uint32_t syshal_pmu_get_startup_status(void)
{
    return reset_reason;
}
