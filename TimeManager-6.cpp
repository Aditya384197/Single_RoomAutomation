#include "TimeManager.h"
#include "Config.h"
#include "StorageManager.h"
#include <WiFi.h>

static bool ntpSynced = false;
static bool fallbackApplied = false;
static unsigned long lastSyncAttemptMs = 0;
static unsigned long lastGoodSyncMs = 0;

static bool trySync(uint32_t timeoutMs) {
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);
  struct tm t;
  if (getLocalTime(&t, timeoutMs)) {
    ntpSynced = true;
    lastGoodSyncMs = millis();
    storage_saveLastEpoch((uint32_t)time(nullptr));
    return true;
  }
  return false;
}

void timeManager_begin() {
  lastSyncAttemptMs = millis();

  uint32_t savedEpoch = storage_loadLastEpoch();
  if (savedEpoch > 0) {
    struct timeval tv;
    tv.tv_sec = (time_t)savedEpoch;
    tv.tv_usec = 0;
    settimeofday(&tv, nullptr);
    fallbackApplied = true;
  }
}

void timeManager_loop() {
  unsigned long now = millis();

  if (!ntpSynced) {
    if (WiFi.status() == WL_CONNECTED && (now - lastSyncAttemptMs > NTP_RETRY_INTERVAL_MS)) {
      lastSyncAttemptMs = now;
      trySync(NTP_SYNC_TIMEOUT_MS);
    }
    return;
  }

  if (WiFi.status() == WL_CONNECTED && (now - lastGoodSyncMs > NTP_RESYNC_INTERVAL_MS)) {
    trySync(NTP_SYNC_TIMEOUT_MS);
  }
}

bool timeManager_isSynced() {
  return ntpSynced || fallbackApplied;
}

bool timeManager_isNtpSynced() {
  return ntpSynced;
}

bool timeManager_getLocalTime(struct tm &outTime) {
  if (!timeManager_isSynced()) return false;
  return getLocalTime(&outTime, 20);
}

String timeManager_getTimeString() {
  struct tm t;
  if (!timeManager_getLocalTime(t)) return String("--:--:--");
  char buf[9];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
  return String(buf);
}

String timeManager_getDateString() {
  struct tm t;
  if (!timeManager_getLocalTime(t)) return String("----:--:--");
  char buf[11];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
  return String(buf);
}

int timeManager_getDayOfYear() {
  struct tm t;
  if (!timeManager_getLocalTime(t)) return -1;
  return t.tm_yday;
}

uint32_t timeManager_getEpoch() {
  if (!timeManager_isSynced()) return 0;
  return (uint32_t)time(nullptr);
}
