/******************************************************************************************
 * File:        syshal_screen.cpp
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

#include "../syshal_screen.h"
#include "../syshal_gpio.h"
#include "../syshal_rtc.h"
#include "../syshal_time.h"
#include "../../core/config/version.h"
#include "../../core/debug/debug.h"
#include "../syshal_config.h"

static volatile bool new_usr_down_pending = false;
static volatile bool new_usr_middle_pending = false;
static volatile bool new_usr_up_pending = false;

static syshal_screen_state_t state = SYSHAL_SCREEN_STATE_UNINIT;

#ifdef WITH_SCREEN
#include <Adafruit_GFX.h>
#include "Adafruit_PCD8544.h"

#define SYSHAL_SCREEN_GPIO_BUTTON_DOWN (GPIO_BUTTON_DOWN)
#define SYSHAL_SCREEN_GPIO_BUTTON_MIDDLE (GPIO_BUTTON_MIDDLE)
#define SYSHAL_SCREEN_GPIO_BUTTON_UP (GPIO_BUTTON_UP)
#define SYSHAL_SCREEN_GPIO_LCD_CLK (GPIO_LCD_CLK)
#define SYSHAL_SCREEN_GPIO_LCD_DIN (GPIO_LCD_DIN)
#define SYSHAL_SCREEN_GPIO_LCD_DC (GPIO_LCD_DC)
#define SYSHAL_SCREEN_GPIO_LCD_CE (GPIO_LCD_CE)
#define SYSHAL_SCREEN_GPIO_LCD_RST (GPIO_LCD_RST)
#define SYSHAL_SCREEN_GPIO_LCD_BL (GPIO_LCD_BL)

static syshal_screen_config_t config;
static syshal_screen_status_t status;
Adafruit_PCD8544 display = Adafruit_PCD8544(SYSHAL_SCREEN_GPIO_LCD_CLK,
                                            SYSHAL_SCREEN_GPIO_LCD_DIN,
                                            SYSHAL_SCREEN_GPIO_LCD_DC,
                                            SYSHAL_SCREEN_GPIO_LCD_CE,
                                            SYSHAL_SCREEN_GPIO_LCD_RST);

// https://javl.github.io/image2cpp/
const unsigned char astrocast_logo16_glcd_bmp[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x37, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78,
    0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00,
    0x80, 0x00, 0x40, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x07, 0x01, 0x03, 0xf0, 0x20, 0x00, 0x00,
    0x01, 0xc0, 0x3e, 0x0f, 0x9f, 0xcf, 0x87, 0xf8, 0x3e, 0x0f, 0x81, 0xc7, 0xf0, 0x7f, 0x1f, 0xdf,
    0xdf, 0xcf, 0x3c, 0x3f, 0x1f, 0xc7, 0xf7, 0xf0, 0xe3, 0x39, 0xc7, 0x0f, 0x9c, 0x1e, 0x73, 0xb8,
    0xce, 0x71, 0xc0, 0x03, 0xbc, 0x07, 0x0e, 0x18, 0x0e, 0x73, 0x80, 0xef, 0x01, 0xc0, 0x7f, 0x9f,
    0x87, 0x0e, 0x38, 0x06, 0xe0, 0x0f, 0xe7, 0xe1, 0xc0, 0xf3, 0x8f, 0xc7, 0x0e, 0x38, 0x06, 0xe0,
    0x3d, 0xe3, 0xf1, 0xc0, 0xe3, 0x81, 0xe7, 0x0e, 0x18, 0x0e, 0xf3, 0xb8, 0xe0, 0x79, 0xc0, 0xe7,
    0xb8, 0xe7, 0x0e, 0x1c, 0x1e, 0x73, 0xb9, 0xee, 0x39, 0xc0, 0xff, 0x9f, 0xc7, 0xce, 0x0f, 0x3c,
    0x7f, 0x3f, 0xe7, 0xf1, 0xf0, 0x7b, 0x8f, 0x83, 0xce, 0x07, 0xf8, 0x3e, 0x1e, 0xe3, 0xe0, 0xf0,
    0x00, 0x00, 0x00, 0x01, 0x03, 0xf0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
    0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x80, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x18, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xf8, 0x00, 0x00, 0x00,
    0x00, 0x00};

char const *menuTxt[] = {"Status       ",
                         "Shutdown     ",
                         "TX msg. list ",
                         "RX msg. list ",
                         "Preset msg. 1",
                         "Preset msg. 2",
                         "Preset msg. 3",
                         "Preset msg. 4",
                         "Preset msg. 5",
                         "Update geoloc",
                         "Clear ALL msg",
                         "Firmware info"};

char const *presetMsg[] = {"Truck stuck with flat tire",
                           "Truck stuck without fuel",
                           "Truck stuck with engine broken",
                           "Will stay outside for the night",
                           "OK, well received"};

#define SYSHAL_SCREEN_WIDTH 84
#define SYSHAL_SCREEN_HIGH 4

byte scr[SYSHAL_SCREEN_WIDTH * SYSHAL_SCREEN_HIGH]; // frame buffer
byte scrWd = SYSHAL_SCREEN_WIDTH;
byte scrHt = SYSHAL_SCREEN_HIGH;
int encoderPos = 0;
int numScrLines = 6;
int numMenus = 0;
int menuLine;
int menuStart;
int menuMode = SYSHAL_SCREEN_MENU_MODE_HOME_PAGE;
int oldPos = 0;
char buf[25], buf2[15];

#endif

// ISR
static void syshal_screen_int_pin_usr_down_priv(void);
static void syshal_screen_int_pin_usr_middle_priv(void);
static void syshal_screen_int_pin_usr_up_priv(void);

// Callback
void syshal_screen_callback_status_page_priv();
void syshal_screen_callback_shutdown_page_priv();
void syshal_screen_callback_tx_msg_list_page_priv();
void syshal_screen_callback_rx_msg_list_page_priv();
void syshal_screen_callback_preset_msg_page_priv(syshal_screen_preset_msg_id_t id);
void syshal_screen_callback_update_geoloc_page_priv();
void syshal_screen_callback_clear_all_user_msg_page_priv();
void syshal_screen_callback_firmware_info_page_priv();

// Graphics
void syshal_screen_frame_buffer_clear();
void syshal_screen_menu_set(int m);
void syshal_screen_menu_end();
void syshal_screen_menu_format(char *in, char *out, int num);
void syshal_screen_draw_menu_slider_priv();

// User inputs
int syshal_screen_read_button();
static void syshal_screen_int_pin_usr_down_priv(void);
static void syshal_screen_int_pin_usr_middle_priv(void);
static void syshal_screen_int_pin_usr_up_priv(void);

static void syshal_screen_int_pin_usr_down_priv(void)
{
    new_usr_down_pending = true;

#if defined(NRF52_SERIES)
    resumeLoop();
#endif
}

static void syshal_screen_int_pin_usr_middle_priv(void)
{
    new_usr_middle_pending = true;

#if defined(NRF52_SERIES)
    resumeLoop();
#endif
}

static void syshal_screen_int_pin_usr_up_priv(void)
{
    new_usr_up_pending = true;

#if defined(NRF52_SERIES)
    resumeLoop();
#endif
}

void syshal_screen_frame_buffer_clear()
{
#ifdef WITH_SCREEN
    for (int i = 0; i < scrWd * scrHt; i++)
        scr[i] = 0;
#else
    DEBUG_PR_ERROR("No supported screen. %s()", __FUNCTION__);
#endif
}

void syshal_screen_menu_set(int m)
{
#ifdef WITH_SCREEN
    menuMode = m;
    display.clearDisplay();
    oldPos = encoderPos;
    encoderPos = 0;
#else
    DEBUG_PR_ERROR("No supported screen. %s()", __FUNCTION__);
#endif
}

int syshal_screen_read_button()
{
    bool state = false;
#ifdef WITH_SCREEN
    if (new_usr_middle_pending)
    {
        state = true;
        new_usr_middle_pending = false;
    }
#else
    DEBUG_PR_ERROR("No supported screen. %s()", __FUNCTION__);
#endif
    return state;
}

void syshal_screen_menu_end()
{
#ifdef WITH_SCREEN
    if (syshal_screen_read_button() > 0)
    {
        menuMode = SYSHAL_SCREEN_MENU_MODE_HOME_PAGE;
        display.clearDisplay();
        encoderPos = oldPos;
    }
#else
    DEBUG_PR_ERROR("No supported screen. %s()", __FUNCTION__);
#endif
}

void syshal_screen_menu_format(char *in, char *out, int num)
{
#ifdef WITH_SCREEN
    int j = strlen(in);
    out[0] = ' ';
    strncpy(out + 1, in, j++);
    for (; j < num; j++)
        out[j] = ' ';
    out[j] = 0;
#else
    DEBUG_PR_ERROR("No supported screen. %s()", __FUNCTION__);
#endif
}

void syshal_screen_draw_menu_slider_priv()
{
#ifdef WITH_SCREEN
    int y, n = (8 * numScrLines - 2 - 5 - 2) * menuLine / (numMenus - 1);
    scrWd = 3;
    scrHt = numScrLines;
    syshal_screen_frame_buffer_clear();
    for (y = 0; y < numScrLines * 8; y++)
        display.drawPixel(1, y, 1);
    for (y = 0; y < 5; y++)
    {
        display.drawPixel(0, y + n + 2, 1);
        display.drawPixel(2, y + n + 2, 1);
    }
#else
    DEBUG_PR_ERROR("No supported screen. %s()", __FUNCTION__);
#endif
}

void syshal_screen_callback_preset_msg_page_priv(syshal_screen_preset_msg_id_t id)
{
#ifdef WITH_SCREEN
    display.clearDisplay();
    if (encoderPos >= 1 * 2)
        encoderPos = 1 * 2;
    int st = encoderPos / 2;
    display.drawRoundRect(0, 0, 84, 36, 3, BLACK);
    display.setCursor(0, 3);
    display.println(presetMsg[id]);
    display.setCursor(4, 39);
    display.print("CANCEL");
    display.setCursor(46, 39);
    display.println(" SEND ");
    if (st == 0)
        display.drawRect(2, 37, 39, 11, BLACK); // CANCEL
    else
        display.drawRect(44, 37, 39, 11, BLACK); // SEND
    if (syshal_screen_read_button() <= 0)
        return;
    menuMode = SYSHAL_SCREEN_MENU_MODE_HOME_PAGE;
    display.clearDisplay();
    if (st > 0)
    {
        display.drawRoundRect(0, 0, 84, 48, 3, BLACK);
        display.setCursor(84 / 2 - 30, 48 / 2 - 4);
        display.print("Queuing...");
        display.display();
        syshal_time_delay_ms(config.screen->contents.page_conf_duration_ms);
        display.clearDisplay();

        syshal_screen_event_t event;
        sprintf((char *)event.preset_msg.buffer, "%s", presetMsg[id]);
        event.preset_msg.buffer_size = strlen((char *)event.preset_msg.buffer);
        event.preset_msg.timestamp = syshal_rtc_return_timestamp();
        event.id = SYSHAL_SCREEN_EVENT_PRESET_MSG;
        syshal_screen_callback(&event);
    }
    encoderPos = oldPos;
#else
    DEBUG_PR_ERROR("No supported screen. %s()", __FUNCTION__);
#endif
}

void syshal_screen_callback_status_page_priv()
{
#ifdef WITH_SCREEN
    // Draw canevas
    display.clearDisplay();
    display.drawRoundRect(0, 9, 84, 30, 3, BLACK);
    // display.drawFastVLine(55, 9, 30, BLACK);

    // Display time
    display.setCursor(0, 0);
    time_t tnow = syshal_rtc_return_timestamp();

    int hours = gmtime(&tnow)->tm_hour;
    int minutes = gmtime(&tnow)->tm_min;

    if (hours < 10)
    {
        display.print("0");
    }
    display.print(hours);
    display.print(":");
    if (minutes < 10)
    {
        display.print("0");
    }
    display.print(minutes);

    // Update status
    syshal_screen_event_t event;
    event.id = SYSHAL_SCREEN_EVENT_UPDATE_STATUS_REQUEST;
    syshal_screen_callback(&event);

    // Display battery voltage
    display.setCursor(60, 0);
    display.print((float)(status.v_bat) / 10, 1);
    display.print("V");

    // Display temperature
    display.setCursor(40, 0);
    display.print(status.temp, 1);
    display.print("C");

    // Display logger counters
    display.setCursor(0, 12);
    display.print(" MSG ");
    display.println(status.u_msg_cnt);
    display.print(" CMD ");
    display.println(status.u_cmd_cnt);
    display.print(" PVT ");
    display.println(status.pvt_cnt);

    // Display location
    display.setCursor(0, 41);
    display.print("N");
    display.print((float)(status.last_loc_lat) * 1E-7, 3);
    display.print(" ");
    display.print("E");
    display.println((float)(status.last_loc_lon) * 1E-7, 3);
#else
    DEBUG_PR_ERROR("No supported screen. %s()", __FUNCTION__);
#endif
}

void syshal_screen_callback_firmware_info_page_priv()
{
#ifdef WITH_SCREEN
    display.clearDisplay();
    display.drawRoundRect(0, 0, 84, 48, 3, BLACK);
    display.setCursor(0, 3);
    display.println(COMPILE_DATE);
    display.println(COMPILE_TIME);
    display.println(COMPILER_NAME);
#else
    DEBUG_PR_ERROR("No supported screen. %s()", __FUNCTION__);
#endif
}

void syshal_screen_callback_shutdown_page_priv()
{
#ifdef WITH_SCREEN
    display.clearDisplay();
    if (encoderPos >= 1 * 2)
        encoderPos = 1 * 2;
    int st = encoderPos / 2;
    display.drawRoundRect(0, 0, 84, 36, 3, BLACK);
    display.setCursor(10, 15);
    display.println("Shut down ?");
    display.setCursor(4, 39);
    display.println("  NO");
    display.setCursor(46, 39);
    display.print(" YES");
    if (st == 0)
        display.drawRect(2, 37, 38, 11, BLACK);
    else
        display.drawRect(44, 37, 38, 11, BLACK);
    if (syshal_screen_read_button() <= 0)
        return;
    menuMode = SYSHAL_SCREEN_MENU_MODE_HOME_PAGE;
    display.clearDisplay();
    if (st > 0)
    {
        display.drawRoundRect(0, 0, 84, 48, 3, BLACK);
        display.setCursor(0, 48 / 2 - 4);
        display.print("Shutting down.");
        display.display();
        syshal_time_delay_ms(config.screen->contents.page_conf_duration_ms);
        display.clearDisplay();

        syshal_screen_event_t event;
        event.id = SYSHAL_SCREEN_EVENT_SHUTDOWN;
        syshal_screen_callback(&event);
    }

    encoderPos = oldPos;
#else
    DEBUG_PR_ERROR("No supported screen. %s()", __FUNCTION__);
#endif
}

void syshal_screen_callback_tx_msg_list_page_priv()
{
#ifdef WITH_SCREEN
    display.clearDisplay();
    display.drawRoundRect(0, 0, 84, 48, 3, BLACK);
    display.setCursor(0, 48 / 2 - 4);
    display.print("   No data.   ");
#else
    DEBUG_PR_ERROR("No supported screen. %s()", __FUNCTION__);
#endif
}

void syshal_screen_callback_rx_msg_list_page_priv()
{
#ifdef WITH_SCREEN
    display.clearDisplay();
    display.drawRoundRect(0, 0, 84, 48, 3, BLACK);
    display.setCursor(0, 48 / 2 - 4);
    display.print("   No data.   ");
#else
    DEBUG_PR_ERROR("No supported screen. %s()", __FUNCTION__);
#endif
}

void syshal_screen_callback_update_geoloc_page_priv()
{
#ifdef WITH_SCREEN
    display.clearDisplay();
    if (encoderPos >= 1 * 2)
        encoderPos = 1 * 2;
    int st = encoderPos / 2;
    display.drawRoundRect(0, 0, 84, 36, 3, BLACK);
    display.setCursor(0, 15);
    display.println("Update GNSS?");
    display.setCursor(4, 39);
    display.println("  NO");
    display.setCursor(46, 39);
    display.print(" YES");
    if (st == 0)
        display.drawRect(2, 37, 38, 11, BLACK);
    else
        display.drawRect(44, 37, 38, 11, BLACK);
    if (syshal_screen_read_button() <= 0)
        return;
    menuMode = SYSHAL_SCREEN_MENU_MODE_HOME_PAGE;
    display.clearDisplay();
    if (st > 0)
    {
        display.drawRoundRect(0, 0, 84, 48, 3, BLACK);
        display.setCursor(0, 48 / 2 - 4);
        display.print(" Updating...");
        display.display();
        syshal_time_delay_ms(config.screen->contents.page_conf_duration_ms);
        display.clearDisplay();

        syshal_screen_event_t event;
        event.id = SYSHAL_SCREEN_EVENT_UPDATE_GEOLOC;
        syshal_screen_callback(&event);
    }
    encoderPos = oldPos;
#else
    DEBUG_PR_ERROR("No supported screen. %s()", __FUNCTION__);
#endif
}

void syshal_screen_callback_clear_all_user_msg_page_priv()
{
#ifdef WITH_SCREEN
    display.clearDisplay();
    if (encoderPos >= 1 * 2)
        encoderPos = 1 * 2;
    int st = encoderPos / 2;
    display.drawRoundRect(0, 0, 84, 36, 3, BLACK);
    display.setCursor(0, 15);
    display.println("Clear all msg?");
    display.setCursor(4, 39);
    display.println("  NO");
    display.setCursor(46, 39);
    display.print(" YES");
    if (st == 0)
        display.drawRect(2, 37, 38, 11, BLACK); // NO
    else
        display.drawRect(44, 37, 38, 11, BLACK); // YES
    if (syshal_screen_read_button() <= 0)
        return;
    menuMode = SYSHAL_SCREEN_MENU_MODE_HOME_PAGE;
    display.clearDisplay();
    if (st > 0)
    {
        display.drawRoundRect(0, 0, 84, 48, 3, BLACK);
        display.setCursor(0, 48 / 2 - 4);
        display.print("  Clearing... ");
        display.display();
        syshal_time_delay_ms(config.screen->contents.page_conf_duration_ms);
        display.clearDisplay();

        syshal_screen_event_t event;
        event.id = SYSHAL_SCREEN_EVENT_CLEAR_ALL_USER_MSG;
        syshal_screen_callback(&event);
    }
    encoderPos = oldPos;
#else
    DEBUG_PR_ERROR("No supported screen. %s()", __FUNCTION__);
#endif
}

int syshal_screen_init(void)
{
#ifdef WITH_SCREEN
    if (state == SYSHAL_SCREEN_STATE_ASLEEP)
        return SYSHAL_SCREEN_ERROR_INVALID_STATE;

    // Configure GPIOs
    syshal_gpio_init(SYSHAL_SCREEN_GPIO_LCD_BL, OUTPUT);

    syshal_gpio_enable_interrupt(SYSHAL_SCREEN_GPIO_BUTTON_DOWN, syshal_screen_int_pin_usr_down_priv, CHANGE);
    syshal_gpio_enable_interrupt(SYSHAL_SCREEN_GPIO_BUTTON_MIDDLE, syshal_screen_int_pin_usr_middle_priv, RISING);
    syshal_gpio_enable_interrupt(SYSHAL_SCREEN_GPIO_BUTTON_UP, syshal_screen_int_pin_usr_up_priv, CHANGE);

    // Try establish connection
    if (syshal_screen_wake_up())
    {
        DEBUG_PR_ERROR("Not detected at default SPI port. Please check wiring. %s()", __FUNCTION__);
        return SYSHAL_SCREEN_ERROR_DEVICE;
    }

    display.clearDisplay(); // clears the screen and buffer
    display.setRotation(2); // rotate 90 degrees counter clockwise, can also use values of 2 and 3 to go further.

    // Display company logo
    display.drawBitmap(0, 8, astrocast_logo16_glcd_bmp, 84, 22, BLACK);
    display.setTextSize(1);
    display.setTextColor(BLACK);
    display.setCursor(6, 33);
    display.println("AstroTracker");
    display.display();

    numMenus = sizeof(menuTxt) / sizeof(char *);
#else
    DEBUG_PR_ERROR("No supported screen. %s()", __FUNCTION__);
#endif

    return SYSHAL_SCREEN_NO_ERROR;
}

int syshal_screen_update_config(syshal_screen_config_t screen_config)
{
#ifdef WITH_SCREEN
    if (state == SYSHAL_SCREEN_STATE_ASLEEP)
        return SYSHAL_SCREEN_ERROR_INVALID_STATE;

    DEBUG_PR_TRACE("Update configuration. %s()", __FUNCTION__);

    config = screen_config;

    if (config.screen->hdr.set)
    {
        // Set contrast
        display.setContrast(config.screen->contents.lcd_contrast);
    }
#else
    DEBUG_PR_ERROR("No supported screen. %s()", __FUNCTION__);
#endif

    return SYSHAL_SCREEN_NO_ERROR;
}

int syshal_screen_term(void)
{
    // Empty

    return SYSHAL_SCREEN_NO_ERROR;
}

int syshal_screen_shutdown(void)
{
#ifdef WITH_SCREEN
    if (state == SYSHAL_SCREEN_STATE_ASLEEP)
        return SYSHAL_SCREEN_NO_ERROR; // SCREEN is already shutdown

    display.clearDisplay();
    display.shutdown();
    syshal_gpio_set_output_low(SYSHAL_SCREEN_GPIO_LCD_BL);

    DEBUG_PR_TRACE("Shutdown. %s()", __FUNCTION__);

    state = SYSHAL_SCREEN_STATE_ASLEEP;

    syshal_screen_event_t event;
    event.id = SYSHAL_SCREEN_EVENT_DISPLAY_OFF;
    syshal_screen_callback(&event);
#else
    DEBUG_PR_ERROR("No supported screen. %s()", __FUNCTION__);
#endif

    return SYSHAL_SCREEN_NO_ERROR;
}

int syshal_screen_wake_up(void)
{
#ifdef WITH_SCREEN
    if (state == SYSHAL_SCREEN_STATE_DISPLAYING)
        return SYSHAL_SCREEN_NO_ERROR; // SCREEN is already awake

    DEBUG_PR_TRACE("Wakeup. %s()", __FUNCTION__);

    syshal_gpio_set_output_high(SYSHAL_SCREEN_GPIO_LCD_BL);

    if (!display.begin())
        return SYSHAL_SCREEN_ERROR_DEVICE;

    state = SYSHAL_SCREEN_STATE_DISPLAYING;

    syshal_screen_event_t event;
    event.id = SYSHAL_SCREEN_EVENT_DISPLAY_ON;
    syshal_screen_callback(&event);
#else
    DEBUG_PR_ERROR("No supported screen. %s()", __FUNCTION__);
#endif

    return SYSHAL_SCREEN_NO_ERROR;
}

void syshal_screen_set_status(syshal_screen_status_t screen_status)
{
#ifdef WITH_SCREEN
    status = screen_status;
#else
    DEBUG_PR_ERROR("No supported screen. %s()", __FUNCTION__);
#endif
}

int syshal_screen_tick(void)
{
#ifdef WITH_SCREEN
    if (state == SYSHAL_SCREEN_STATE_UNINIT)
        return SYSHAL_SCREEN_ERROR_INVALID_STATE;

    if (new_usr_down_pending)
    {
        new_usr_down_pending = false;
        encoderPos--;

        syshal_screen_event_t event;
        event.id = SYSHAL_SCREEN_EVENT_BUTTON_PRESSED;
        syshal_screen_callback(&event);
    }
    else if (new_usr_up_pending)
    {
        new_usr_up_pending = false;
        encoderPos++;

        syshal_screen_event_t event;
        event.id = SYSHAL_SCREEN_EVENT_BUTTON_PRESSED;
        syshal_screen_callback(&event);
    }

    if (encoderPos < 0)
        encoderPos = 0;

    if (state == SYSHAL_SCREEN_STATE_ASLEEP)
        return SYSHAL_SCREEN_NO_ERROR;

    DEBUG_PR_TRACE("Process EVENT... %s()", __FUNCTION__);

    if (menuMode == SYSHAL_SCREEN_MENU_MODE_HOME_PAGE)
    {
        display.clearDisplay();
        menuLine = encoderPos / 2;
        if (menuLine >= numMenus)
        {
            menuLine = numMenus - 1;
            encoderPos = menuLine * 2;
        }
        if (menuLine >= menuStart + numScrLines)
            menuStart = menuLine - numScrLines + 1;
        if (menuLine < menuStart)
            menuStart = menuLine;
        for (int i = 0; i < numScrLines; i++)
        {
            if (i + menuStart < numMenus)
            {
                syshal_screen_menu_format((char *)menuTxt[i + menuStart], buf, 14);
                display.printf(buf);
                if (i + menuStart == menuLine)
                {
                    display.drawRect(4, 8 * (i), 80, 8, BLACK);
                }
            }
        }
        syshal_screen_draw_menu_slider_priv();
        if (syshal_screen_read_button())
        {
            syshal_screen_menu_set(menuLine);
        }
    }
    else if (menuMode == SYSHAL_SCREEN_MENU_MODE_STATUS_PAGE)
    {
        syshal_screen_callback_status_page_priv();
        syshal_screen_menu_end();
    }
    else if (menuMode == SYSHAL_SCREEN_MENU_MODE_SHUTDOWN_PAGE)
    {
        syshal_screen_callback_shutdown_page_priv();
        syshal_screen_menu_end();
    }
    else if (menuMode == SYSHAL_SCREEN_MENU_MODE_TX_MSG_LST_PAGE)
    {
        syshal_screen_callback_tx_msg_list_page_priv();
        syshal_screen_menu_end();
    }
    else if (menuMode == SYSHAL_SCREEN_MENU_MODE_RX_MSG_LST_PAGE)
    {
        syshal_screen_callback_rx_msg_list_page_priv();
        syshal_screen_menu_end();
    }
    else if (menuMode == SYSHAL_SCREEN_MENU_MODE_PRESET_MSG_1)
    {
        syshal_screen_callback_preset_msg_page_priv(SYSHAL_SCREEN_PRESET_MSG_1);
        syshal_screen_menu_end();
    }
    else if (menuMode == SYSHAL_SCREEN_MENU_MODE_PRESET_MSG_2)
    {
        syshal_screen_callback_preset_msg_page_priv(SYSHAL_SCREEN_PRESET_MSG_2);
        syshal_screen_menu_end();
    }
    else if (menuMode == SYSHAL_SCREEN_MENU_MODE_PRESET_MSG_3)
    {
        syshal_screen_callback_preset_msg_page_priv(SYSHAL_SCREEN_PRESET_MSG_3);
        syshal_screen_menu_end();
    }
    else if (menuMode == SYSHAL_SCREEN_MENU_MODE_PRESET_MSG_4)
    {
        syshal_screen_callback_preset_msg_page_priv(SYSHAL_SCREEN_PRESET_MSG_4);
        syshal_screen_menu_end();
    }
    else if (menuMode == SYSHAL_SCREEN_MENU_MODE_PRESET_MSG_5)
    {
        syshal_screen_callback_preset_msg_page_priv(SYSHAL_SCREEN_PRESET_MSG_5);
        syshal_screen_menu_end();
    }
    else if (menuMode == SYSHAL_SCREEN_MENU_MODE_UPDATE_GEOLOC_PAGE)
    {
        syshal_screen_callback_update_geoloc_page_priv();
        syshal_screen_menu_end();
    }
    else if (menuMode == SYSHAL_SCREEN_MENU_MODE_CLEAR_ALL_USER_MSG_PAGE)
    {
        syshal_screen_callback_clear_all_user_msg_page_priv();
        syshal_screen_menu_end();
    }
    else if (menuMode == SYSHAL_SCREEN_MENU_MODE_FIRMWARE_INFO_PAGE)
    {
        syshal_screen_callback_firmware_info_page_priv();
        syshal_screen_menu_end();
    }
    else
    {
        menuMode = SYSHAL_SCREEN_MENU_MODE_HOME_PAGE;
        display.clearDisplay();
    }

    display.display();
#else
    // DEBUG_PR_ERROR("No supported screen. %s()", __FUNCTION__);
#endif

    return SYSHAL_SCREEN_NO_ERROR;
}

syshal_screen_state_t syshal_screen_get_state(void)
{
    return state;
}

__attribute__((weak)) void syshal_screen_callback(syshal_screen_event_t *event)
{
    DEBUG_PR_WARN("%s Not implemented", __FUNCTION__);
}
