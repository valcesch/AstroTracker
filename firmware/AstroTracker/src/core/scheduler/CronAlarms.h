/******************************************************************************************
  CronAlarms.cpp - Arduino cron alarms
  Copyright (c) 2019 Martin Laclaustra

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  This is a wrapper of ccronexpr
  Copyright 2015, alex at staticlibs.net
  Licensed under the Apache License, Version 2.0
  https://github.com/staticlibs/ccronexpr

  API and implementation are inspired in TimeAlarms
  Copyright 2008-2011 Michael Margolis, maintainer:Paul Stoffregen
  GNU Lesser General Public License, Version 2.1 or later
  https://github.com/PaulStoffregen/TimeAlarms
 ******************************************************************************************/

#ifndef CronAlarms_h
#define CronAlarms_h

#include <Arduino.h>
#include <time.h>

extern "C"
{
#include "CronExpr.h"
}

#if !defined(dtNBR_ALARMS)
#if defined(__AVR__)
#define dtNBR_ALARMS 6 // max is 255
#elif defined(ESP8266)
#define dtNBR_ALARMS 20 // for esp8266 chip - max is 255
#else
#define dtNBR_ALARMS 6 // assume non-AVR has more memory
#endif
#endif

#define USE_SPECIALIST_METHODS // define this for testing

typedef uint8_t CronID_t;
typedef CronID_t CronId; // Arduino friendly name

#define dtINVALID_ALARM_ID 255
#define dtINVALID_TIME (time_t)(-1)

typedef void (*OnTick_t)(); // alarm callback function typedef

// class defining an alarm instance, only used by dtAlarmsClass
class CronEventClass
{
public:
  CronEventClass();
  void updateNextTrigger();
  cron_expr expr;
  OnTick_t onTickHandler;
  time_t nextTrigger;
  bool isEnabled; // the timer is only actioned if isEnabled is true
  bool isOneShot; // the timer will be de-allocated after trigger is processed
};

// class containing the collection of alarms
class CronClass
{
private:
  CronEventClass Alarm[dtNBR_ALARMS];
  uint8_t isServicing;
  uint8_t servicedCronId; // the alarm currently being serviced
  void serviceAlarms();

public:
  CronClass(uint32_t (*rtc_getTime)(void));

  // Function to create alarms and timers with cron
  CronID_t create(char *cronstring, OnTick_t onTickHandler, bool isOneShot);
  // isOneShot - trigger once at the given time in the future

  // Function that must be evaluated often (at least once every main loop)
  void delay(unsigned long ms = 0);

  // low level methods
  void enable(CronID_t ID);            // enable the alarm to trigger
  void disable(CronID_t ID);           // prevent the alarm from triggering
  CronID_t getTriggeredCronId() const; // returns the currently triggered  alarm id
  bool getIsServicing() const;         // returns isServicing

  void free(CronID_t ID); // free the id to allow its reuse

#ifndef USE_SPECIALIST_METHODS
private: // the following methods are for testing and are not documented as part of the standard library
#endif
  uint8_t count() const;                    // returns the number of allocated timers
  time_t getNextTrigger(CronID_t *ID) const;            // returns the time of the next scheduled alarm
  time_t getNextTrigger(CronID_t ID) const; // returns the time of scheduled alarm
  bool isAllocated(CronID_t ID) const;      // returns true if this id is allocated
};

#endif /* CronAlarms_h */
