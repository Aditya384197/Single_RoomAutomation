#pragma once
/*
  Two independent one-shot timers per device:
    TIMER_AUTO_ON  - after the set duration, turns the device ON
    TIMER_AUTO_OFF - after the set duration, turns the device OFF
  They do not hold or revert state - each just fires a single action at
  its target time, and can run concurrently with the other kind or with
  the daily schedule.

  Epoch-based (reboot-safe) when a usable clock exists; falls back to a
  millis() countdown for this power session only if no clock is available
  yet, so starting a timer never behaves as if it were already expired.
*/

#include "Config.h"

void timerManager_begin();
void timerManager_loop();

void timerManager_start(DeviceId id, TimerKind kind, uint32_t minutes);
void timerManager_cancel(DeviceId id, TimerKind kind);

// Cancels both AUTO_ON and AUTO_OFF for a device (only if active - avoids
// pointless flash writes). Used when a human directly acts on the device
// (physical switch or dashboard toggle), since a pending timer becomes
// meaningless once someone has manually taken control.
void timerManager_cancelAllForDevice(DeviceId id);

bool     timerManager_isActive(DeviceId id, TimerKind kind);
uint32_t timerManager_remainingSeconds(DeviceId id, TimerKind kind);
uint32_t timerManager_totalSeconds(DeviceId id, TimerKind kind);
