/* exceptions.h - All possible exceptions used by cexception for error handling
 *
 * Copyright (C) 2018 Arribada
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _EXCEPTIONS_H_
#define _EXCEPTIONS_H_

// All error codes
enum
{
    EXCEPTION_BAD_SYS_CONFIG_ERROR_CONDITION,
    EXCEPTION_BLE_ERROR,
    EXCEPTION_GPS_ERROR,
    EXCEPTION_FS_ERROR,
    EXCEPTION_SPI_ERROR,
    EXCEPTION_LOG_BUFFER_FULL,
    EXCEPTION_SAT_ERROR,
    EXCEPTION_BOOT_ERROR,
    EXCEPTION_FLASH_ERROR,
    EXCEPTION_SCHEDULER_ERROR,
    EXCEPTION_SATPASS_ERROR,
    EXCEPTION_SCREEN_ERROR,
};

#endif /* _EXCEPTIONS_H_ */
