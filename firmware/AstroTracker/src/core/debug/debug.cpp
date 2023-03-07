/******************************************************************************************
 * File:        debug.cpp
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

#include "debug.h"

debug_level_t g_debug_level = DEBUG_TRACE;

const char *g_dbg_lvl[] =
    {
        [DEBUG_NONE] = "NONE",
        [DEBUG_SYSTEM] = "SYSTEM",
        [DEBUG_ERROR] = "ERROR",
        [DEBUG_WARN] = "WARN",
        [DEBUG_INFO] = "INFO",
        [DEBUG_TRACE] = "TRACE",
};

#define DEBUG_BAUDRATE 115200

int debug_init(void)
{
    UART_DEBUG.begin(DEBUG_BAUDRATE);

    return DEBUG_NO_ERROR;
}

/*
 * Source: https://gist.github.com/ridencww/4e5d10097fee0b0f7f6b?permalink_comment_id=3617754
 *
 * Simple printf for writing to an Arduino serial port.  Allows specifying Serial..Serial3.
 *
 * const HardwareSerial&, the serial port to use (Serial..Serial3)
 * const char* fmt, the formatting string followed by the data to be formatted
 *
 * int d = 65;
 * float f = 123.4567;
 * char* str = "Hello";
 * serial_printf(Serial, "<fmt>", d);
 *
 * Example:
 *   serial_printf(Serial, "Sensor %d is %o and reads %1f\n", d, d, f) will
 *   output "Sensor 65 is on and reads 123.5" to the serial port.
 *
 * Formatting strings <fmt>
 * %B    - binary (d = 0b1000001)
 * %b    - binary (d = 1000001)
 * %c    - character (s = H)
 * %d/%i - integer (d = 65)\
 * %f    - float (f = 123.45)
 * %3f   - float (f = 123.346) three decimal places specified by %3.
 * %o    - boolean on/off (d = On)
 * %s    - char* string (s = Hello)
 * %X    - hexidecimal (d = 0x41)
 * %x    - hexidecimal (d = 41)
 * %%    - escaped percent ("%")
 * Thanks goes to @alw1746 for his %.4f precision enhancement
 */
void serial_printf(const char *fmt, ...)
{
    va_list argv;
    va_start(argv, fmt);

    for (int i = 0; fmt[i] != '\0'; i++)
    {
        if (fmt[i] == '%')
        {
            // Look for specification of number of decimal places
            int places = 2;
            if (fmt[i + 1] == '.')
                i++; // alw1746: Allows %.4f precision like in stdio printf (%4f will still work).
            if (fmt[i + 1] >= '0' && fmt[i + 1] <= '9')
            {
                places = fmt[i + 1] - '0';
                i++;
            }

            switch (fmt[++i])
            {
            case 'B':
                UART_DEBUG.print("0b"); // Fall through intended
            case 'b':
                UART_DEBUG.print(va_arg(argv, int), BIN);
                break;
            case 'c':
                UART_DEBUG.print((char)va_arg(argv, int));
                break;
            case 'd':
            case 'i':
                UART_DEBUG.print(va_arg(argv, int), DEC);
                break;
            case 'f':
                UART_DEBUG.print(va_arg(argv, double), places);
                break;
            case 'l':
                UART_DEBUG.print(va_arg(argv, long), DEC);
                break;
            case 'o':
                UART_DEBUG.print(va_arg(argv, int) == 0 ? "off" : "on");
                break;
            case 's':
                UART_DEBUG.print(va_arg(argv, const char *));
                break;
            case 'X':
                UART_DEBUG.print("0x"); // Fall through intended
            case 'x':
                UART_DEBUG.print(va_arg(argv, int), HEX);
                break;
            case '%':
                UART_DEBUG.print(fmt[i]);
                break;
            default:
                UART_DEBUG.print("?");
                break;
            }
        }
        else
        {
            UART_DEBUG.print(fmt[i]);
        }
    }
    va_end(argv);
}
