#pragma once
/*
  DeviceControl.h
  ----------------
  Owns the actual relay GPIOs and the physical maintained-toggle-switch
  inputs for Fan and LED.

  Key design point (see chat explanation): the physical switch is treated
  as an EDGE source, not a level/position source. Every confirmed
  (debounced) change in switch position simply flips whatever the current
  relay state is - so dashboard, schedule, timer and physical switch can
  all safely share control of the same relay without ever fighting or
  getting stuck out of sync.
*/

#include <Arduino.h>
#include "Config.h"

void deviceControl_begin();
void deviceControl_loop();   // call every loop() - handles switch debounce

bool deviceControl_getState(DeviceId id);

// source is only used for dashboard display ("manual" / "dashboard" / "schedule" / "timer")
void deviceControl_setState(DeviceId id, bool on, const char* source);

const char* deviceControl_getLastSource(DeviceId id);
