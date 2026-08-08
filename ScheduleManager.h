#pragma once

#include "Config.h"
#include "StorageManager.h"

void scheduleManager_begin();
void scheduleManager_loop();

void scheduleManager_set(DeviceId id, bool enabled,
                          uint8_t onHour, uint8_t onMin,
                          uint8_t offHour, uint8_t offMin);

PersistedSchedule scheduleManager_get(DeviceId id);
