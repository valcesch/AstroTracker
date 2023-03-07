/******************************************************************************************
 * File:        syshal_gpio.cpp
 * Author:      Raphael Valceschini
 * Compagny:    NA
 * Website:     https://github.com/valcesch/AstroTracker
 * E-mail:      NA
 *
 * AstroTracker
 * Copyright (C) 2023 Raphael Valceschini
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
#include "../syshal_config.h"

#if defined(ARDUINO_ARCH_SAMD)
static void configGCLK6()
{
    // Based on https://github.com/arduino-libraries/ArduinoLowPower/blob/master/src/samd/ArduinoLowPower.cpp

    // keep the XOSC32K running in standy
    SYSCTRL->XOSC32K.reg |= SYSCTRL_XOSC32K_RUNSTDBY;

    // enable EIC clock
    GCLK->CLKCTRL.bit.CLKEN = 0; // disable GCLK module
    while (GCLK->STATUS.bit.SYNCBUSY)
        ;

    GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK6 | GCLK_CLKCTRL_ID(GCM_EIC)); // EIC clock switched on GCLK6
    while (GCLK->STATUS.bit.SYNCBUSY)
        ;

    GCLK->GENCTRL.reg = (GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_XOSC32K | GCLK_GENCTRL_ID(6)); // source for GCLK6 is OSCULP32K
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY)
        ;

    GCLK->GENCTRL.bit.RUNSTDBY = 1; // GCLK6 run standby
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY)
        ;
}
#endif

void syshal_gpio_init(uint32_t pin, uint32_t mode)
{
    pinMode(pin, mode);
}

void syshal_gpio_term(uint32_t pin)
{
#if defined(NRF52_SERIES)
    nrf_gpio_cfg_default(pin);
#elif defined(ARDUINO_ARCH_SAMD)
    PORT->Group[g_APinDescription[pin].ulPort].PINCFG[g_APinDescription[pin].ulPin].reg = 0;
#endif
}

int syshal_gpio_enable_interrupt(uint32_t pin, voidFuncPtr callback, irq_mode mode)
{
#if defined(NRF52_SERIES)
    // Empty
#elif defined(ARDUINO_ARCH_SAMD)
    EExt_Interrupts in = g_APinDescription[pin].ulExtInt;
    if (in == NOT_AN_INTERRUPT || in == EXTERNAL_INT_NMI)
        return SYSHAL_GPIO_NO_INTERRUPT_PIN;

    // pinMode(pin, INPUT_PULLUP);
    attachInterrupt(pin, callback, mode);

    configGCLK6();

    // Enable wakeup capability on pin in case being used during sleep
    EIC->WAKEUP.reg |= (1 << in);
#endif

    return SYSHAL_GPIO_NO_ERROR;
}

int syshal_gpio_disable_interrupt(uint32_t pin)
{
    // TODO

    return SYSHAL_GPIO_NO_ERROR;
}

void syshal_gpio_set_output_low(uint32_t pin)
{
    digitalWrite(pin, LOW);
}

void syshal_gpio_set_output_high(uint32_t pin)
{
    digitalWrite(pin, HIGH);
}

void syshal_gpio_set_output_toggle(uint32_t pin)
{
    digitalWrite(pin, !digitalRead(pin));
}

bool syshal_gpio_get_input(uint32_t pin)
{
    return digitalRead(pin);
}

uint32_t syshal_gpio_analog_read(uint32_t pin)
{
    return analogRead(pin);
}