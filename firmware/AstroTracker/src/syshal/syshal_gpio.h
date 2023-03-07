/******************************************************************************************
 * File:        syshal_gpio.h
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

#ifndef _SYSHAL_GPIO_H_
#define _SYSHAL_GPIO_H_

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#ifdef ARDUINO_API_VERSION
using irq_mode = PinStatus;
#else
using irq_mode = uint32_t;
#endif

#define SYSHAL_GPIO_NO_ERROR (0)
#define SYSHAL_GPIO_NO_FREE_INTERRUPT (-1)
#define SYSHAL_GPIO_NO_INTERRUPT_PIN (-2)

typedef enum
{
    SYSHAL_GPIO_EVENT_LOW_TO_HIGH,
    SYSHAL_GPIO_EVENT_HIGH_TO_LOW,
    SYSHAL_GPIO_EVENT_TOGGLE,
} syshal_gpio_event_id_t;

typedef struct
{
    syshal_gpio_event_id_t id;
    uint32_t pin_number;
} syshal_gpio_event_t;

void syshal_gpio_init(uint32_t pin, uint32_t mode);
void syshal_gpio_term(uint32_t pin);
int syshal_gpio_enable_interrupt(uint32_t pin, voidFuncPtr callback, irq_mode mode);
int syshal_gpio_disable_interrupt(uint32_t pin);
void syshal_gpio_set_output_low(uint32_t pin);
void syshal_gpio_set_output_high(uint32_t pin);
void syshal_gpio_set_output_toggle(uint32_t pin);
bool syshal_gpio_get_input(uint32_t pin);
uint32_t syshal_gpio_analog_read(uint32_t pin);

#endif /* _SYSHAL_GPIO_H_ */