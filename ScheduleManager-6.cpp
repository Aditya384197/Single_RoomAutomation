#include "ScheduleManager.h"
#include "TimeManager.h"
#include "DeviceControl.h"
#include <time.h>

struct ScheduleRuntime {
  PersistedSchedule cfg;
  int lastTriggeredOnDay;
  int lastTriggeredOffDay;
  bool everSynced;
};

static ScheduleRuntime sch[DEV_COUNT];
static unsigned long lastCheckMs = 0;

static int toMinutes(uint8_t h, uint8_t m) { return (int)h * 60 + (int)m; }

static bool nowWithinWindow(int nowMin, int onMin, int offMin) {
  if (onMin == offMin) return false;
  if (onMin < offMin) return (nowMin >= onMin && nowMin < offMin);
  return (nowMin >= onMin || nowMin < offMin);
}

static void syncNow(DeviceId id) {
  if (!sch[id].cfg.enabled) return;
  struct tm t;
  if (!timeManager_getLocalTime(t)) return;

  int nowMin = toMinutes(t.tm_hour, t.tm_min);
  int onMin  = toMinutes(sch[id].cfg.onHour, sch[id].cfg.onMin);
  int offMin = toMinutes(sch[id].cfg.offHour, sch[id].cfg.offMin);

  bool shouldBeOn = nowWithinWindow(nowMin, onMin, offMin);

  int today = t.tm_yday;
  if (shouldBeOn) sch[id].lastTriggeredOnDay = today;
  else            sch[id].lastTriggeredOffDay = today;

  deviceControl_setState(id, shouldBeOn, "schedule");
}

void scheduleManager_begin() {
  for (int i = 0; i < DEV_COUNT; i++) {
    sch[i].cfg = storage_loadSchedule((DeviceId)i);
    sch[i].lastTriggeredOnDay = -1;
    sch[i].lastTriggeredOffDay = -1;
    sch[i].everSynced = false;
  }
}

void scheduleManager_set(DeviceId id, bool enabled,
                          uint8_t onHour, uint8_t onMin,
                          uint8_t offHour, uint8_t offMin) {
  sch[id].cfg.enabled = enabled;
  sch[id].cfg.onHour  = onHour  % 24;
  sch[id].cfg.onMin   = onMin   % 60;
  sch[id].cfg.offHour = offHour % 24;
  sch[id].cfg.offMin  = offMin  % 60;
  sch[id].lastTriggeredOnDay = -1;
  sch[id].lastTriggeredOffDay = -1;

  storage_saveSchedule(id, sch[id].cfg);
  syncNow(id);
}

PersistedSchedule scheduleManager_get(DeviceId id) {
  return sch[id].cfg;
}

void scheduleManager_loop() {
  unsigned long now = millis();
  if (now - lastCheckMs < 1000UL) return;
  lastCheckMs = now;

  if (!timeManager_isSynced()) return;

  struct tm t;
  if (!timeManager_getLocalTime(t)) return;

  int nowMin = toMinutes(t.tm_hour, t.tm_min);
  int today = t.tm_yday;

  for (int i = 0; i < DEV_COUNT; i++) {
    DeviceId id = (DeviceId)i;
    if (!sch[i].cfg.enabled) continue;

    if (!sch[i].everSynced) {
      sch[i].everSynced = true;
      syncNow(id);
      continue;
    }

    int onMin  = toMinutes(sch[i].cfg.onHour, sch[i].cfg.onMin);
    int offMin = toMinutes(sch[i].cfg.offHour, sch[i].cfg.offMin);

    if (nowMin == onMin && sch[i].lastTriggeredOnDay != today) {
      sch[i].lastTriggeredOnDay = today;
      deviceControl_setState(id, true, "schedule");
    }
    if (nowMin == offMin && sch[i].lastTriggeredOffDay != today) {
      sch[i].lastTriggeredOffDay = today;
      deviceControl_setState(id, false, "schedule");
    }
  }
}
