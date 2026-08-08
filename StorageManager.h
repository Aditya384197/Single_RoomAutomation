#pragma once

#include <Arduino.h>
#include "Config.h"

struct PersistedSchedule {
  bool    enabled;
  uint8_t onHour;
  uint8_t onMin;
  uint8_t offHour;
  uint8_t offMin;
};

struct PersistedTimer {
  bool     active;
  bool     useEpoch;
  uint32_t endEpoch;
  uint32_t totalSec;
};

struct ApConfig {
  char ssid[33];
  char pass[65];
};

void storage_begin();

void storage_saveDeviceState(DeviceId id, bool state);
bool storage_loadDeviceState(DeviceId id);

void storage_saveSchedule(DeviceId id, const PersistedSchedule &s);
PersistedSchedule storage_loadSchedule(DeviceId id);

void storage_saveTimer(DeviceId id, TimerKind kind, const PersistedTimer &t);
PersistedTimer storage_loadTimer(DeviceId id, TimerKind kind);

void     storage_saveLastEpoch(uint32_t epochSeconds);
uint32_t storage_loadLastEpoch();

void storage_saveApConfig(const ApConfig &cfg);
ApConfig storage_loadApConfig();

void storage_factoryReset();
