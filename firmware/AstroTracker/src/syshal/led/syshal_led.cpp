/******************************************************************************************
 * File:        syshal_led.cpp
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

#include "../../core/debug/debug.h"
#include "../syshal_led.h"
#include "../syshal_time.h"
#include "../syshal_gpio.h"
#include "../syshal_config.h"

#ifdef GPIO_LED_NEOPIXEL
#include "Adafruit_NeoPixel.h"
Adafruit_NeoPixel neopixel;
#endif

#define SYSHAL_LED_GPIO_LED (GPIO_LED)

#define NEOPIXEL_NB_LED 1
#define NEOPIXEL_TYPE NEO_GRB + NEO_KHZ800
#define NEOPIXEL_BRIGHTNESS 50

#define SYSHAL_LED_ON (1)
#define SYSHAL_LED_OFF (0)

#define UINT32_COLOUR_TO_UINT8_RED(x) ((x >> 16) & 0xFF)
#define UINT32_COLOUR_TO_UINT8_GREEN(x) ((x >> 8) & 0xFF)
#define UINT32_COLOUR_TO_UINT8_BLUE(x) (x & 0xFF)

static enum {
    SOLID,
    BLINK,
    SEQUENCE,
    OFF,
} current_type;

static volatile uint32_t current_colour = SYSHAL_LED_COLOUR_OFF;
static volatile uint8_t last_state;
static syshal_led_sequence_t current_sequence;
static uint32_t start_blink_time_ms;
static uint32_t blink_period_ms;

void set_colour(uint32_t colour)
{
#ifdef GPIO_LED_NEOPIXEL
    neopixel.fill(neopixel.Color(UINT32_COLOUR_TO_UINT8_RED(colour),
                                 UINT32_COLOUR_TO_UINT8_GREEN(colour),
                                 UINT32_COLOUR_TO_UINT8_BLUE(colour)));
    neopixel.show();
#endif

    if (colour == SYSHAL_LED_COLOUR_OFF)
    {
#ifdef LED_INVERTED
        syshal_gpio_set_output_high(SYSHAL_LED_GPIO_LED);
#else
        syshal_gpio_set_output_low(SYSHAL_LED_GPIO_LED);
#endif
    }
    else
    {
#ifdef LED_INVERTED
        syshal_gpio_set_output_low(SYSHAL_LED_GPIO_LED);
#else
        syshal_gpio_set_output_high(SYSHAL_LED_GPIO_LED);
#endif
    }
}

int syshal_led_init(void)
{
#ifdef GPIO_LED_NEOPIXEL
    neopixel.begin();
    neopixel.setPin(GPIO_LED_NEOPIXEL);
    neopixel.updateLength(NEOPIXEL_NB_LED);
    neopixel.updateType(NEOPIXEL_TYPE);
    neopixel.setBrightness(NEOPIXEL_BRIGHTNESS);
    neopixel.show();
#endif

    syshal_gpio_init(SYSHAL_LED_GPIO_LED, OUTPUT);

    return SYSHAL_LED_NO_ERROR;
}

int syshal_led_set_solid(uint32_t colour)
{
    current_colour = colour;
    current_type = SOLID;
    set_colour(colour);
    return SYSHAL_LED_NO_ERROR;
}

int syshal_led_set_blinking(uint32_t colour, uint32_t time_ms)
{
    current_colour = colour;
    current_type = BLINK;

    last_state = SYSHAL_LED_ON;

    blink_period_ms = time_ms;
    start_blink_time_ms = syshal_time_get_ticks_ms();
    set_colour(colour);

    return SYSHAL_LED_NO_ERROR;
}

int syshal_led_get(uint32_t *colour, bool *is_blinking)
{
    if (current_type == OFF)
        return SYSHAL_LED_ERROR_LED_OFF;

    if (*colour)
        *colour = current_colour;

    if (is_blinking)
        *is_blinking = (current_type == BLINK);

    return SYSHAL_LED_NO_ERROR;
}

int syshal_led_set_sequence(syshal_led_sequence_t sequence, uint32_t time_ms)
{
    switch (sequence)
    {
    case RED_GREEN_BLUE:
        current_type = SEQUENCE;
        current_sequence = RED_GREEN_BLUE;
        current_colour = SYSHAL_LED_COLOUR_RED;

        blink_period_ms = time_ms;
        start_blink_time_ms = syshal_time_get_ticks_ms();

        set_colour(current_colour);
        break;

    default:
        return SYSHAL_LED_ERROR_SEQUENCE_NOT_DEFINE;
        break;
    }

    return SYSHAL_LED_NO_ERROR;
}

int syshal_led_off(void)
{
    current_type = OFF;
    set_colour(SYSHAL_LED_COLOUR_OFF);

    return SYSHAL_LED_NO_ERROR;
}

bool syshal_led_is_active(void)
{
    return (current_type != OFF);
}

void syshal_led_tick(void)
{
    if (current_type == OFF ||
        current_type == SOLID)
        return;

    if (syshal_time_get_ticks_ms() - start_blink_time_ms >= blink_period_ms)
    {
        if (current_type == BLINK)
        {
            if (last_state == SYSHAL_LED_ON)
            {
                last_state = SYSHAL_LED_OFF;
                set_colour(SYSHAL_LED_COLOUR_OFF);
            }
            else
            {
                last_state = SYSHAL_LED_ON;
                set_colour(current_colour);
            }
        }
        else if (current_type == SEQUENCE)
        {
            switch (current_sequence)
            {
            case RED_GREEN_BLUE:
                switch (current_colour)
                {
                case SYSHAL_LED_COLOUR_RED:
                    current_colour = SYSHAL_LED_COLOUR_GREEN;
                    break;
                case SYSHAL_LED_COLOUR_GREEN:
                    current_colour = SYSHAL_LED_COLOUR_BLUE;
                    break;
                case SYSHAL_LED_COLOUR_BLUE:
                    current_colour = SYSHAL_LED_COLOUR_RED;
                    break;
                default:
                    current_colour = SYSHAL_LED_COLOUR_RED;
                    break;
                }
                set_colour(current_colour);
                break;
            default:
                break;
            }
        }

        start_blink_time_ms = syshal_time_get_ticks_ms();
    }
}