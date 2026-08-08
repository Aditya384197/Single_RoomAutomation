#pragma once
/*
  Two-tier time validity:
    timeManager_isSynced()    - true once ANY usable time exists, either a
                                 real NTP sync or a fallback time loaded
                                 from flash. Schedules/timers gate on this.
    timeManager_isNtpSynced() - true only after a real NTP sync this boot.

  On every successful NTP sync the current epoch is saved to flash, so a
  reboot without internet still starts from a recent time instead of 1970.
*/

#include <Arduino.h>
#include <time.h>

void   timeManager_begin();
void   timeManager_loop();
bool   timeManager_isSynced();
bool   timeManager_isNtpSynced();
bool   timeManager_getLocalTime(struct tm &outTime);
String timeManager_getTimeString();
String timeManager_getDateString();
int    timeManager_getDayOfYear();
uint32_t timeManager_getEpoch();
