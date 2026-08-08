#include "StorageManager.h"
#include <Preferences.h>
#include <nvs_flash.h>
#include <string.h>

static Preferences prefs;

static const char* stateKey(DeviceId id) { return (id == DEV_FAN) ? "st_fan" : "st_led"; }
static const char* schedKey(DeviceId id) { return (id == DEV_FAN) ? "sc_fan" : "sc_led"; }
static const char* EPOCH_KEY = "last_epoch";
static const char* AP_CONFIG_KEY = "ap_cfg";

static const char* timerKey(DeviceId id, TimerKind kind) {
  if (id == DEV_FAN) return (kind == TIMER_AUTO_ON) ? "tm_fan_on" : "tm_fan_of";
  return (kind == TIMER_AUTO_ON) ? "tm_led_on" : "tm_led_of";
}

void storage_begin() {
  if (prefs.begin(NVS_NAMESPACE, false)) return;

  Serial.println("[STORAGE] NVS open failed, retrying...");
  delay(200);
  if (prefs.begin(NVS_NAMESPACE, false)) return;

  // Still failing - the partition may be corrupted (e.g. after a firmware
  // update changed the NVS layout). Try a full erase and reinit once. This
  // wipes all saved settings, but a working-but-reset device is far better
  // than one silently running with no persistence, or one stuck endlessly
  // rebooting because a genuinely bad partition can't fix itself that way.
  Serial.println("[STORAGE] NVS still failing, attempting erase and reinit...");
  nvs_flash_erase();
  nvs_flash_init();
  if (!prefs.begin(NVS_NAMESPACE, false)) {
    Serial.println("[STORAGE] WARNING: NVS unavailable - running without persistence this session.");
  }
}

void storage_saveDeviceState(DeviceId id, bool state) {
  if (prefs.putBool(stateKey(id), state) == 0) {
    Serial.println("[STORAGE] WARNING: failed to save device state");
  }
}

bool storage_loadDeviceState(DeviceId id) {
  return prefs.getBool(stateKey(id), false);
}

void storage_saveSchedule(DeviceId id, const PersistedSchedule &s) {
  if (prefs.putBytes(schedKey(id), &s, sizeof(PersistedSchedule)) != sizeof(PersistedSchedule)) {
    Serial.println("[STORAGE] WARNING: failed to save schedule");
  }
}

PersistedSchedule storage_loadSchedule(DeviceId id) {
  PersistedSchedule s = {false, 0, 0, 0, 0};
  if (prefs.isKey(schedKey(id))) {
    prefs.getBytes(schedKey(id), &s, sizeof(PersistedSchedule));
  }
  return s;
}

void storage_saveTimer(DeviceId id, TimerKind kind, const PersistedTimer &t) {
  const char* key = timerKey(id, kind);
  if (prefs.putBytes(key, &t, sizeof(PersistedTimer)) != sizeof(PersistedTimer)) {
    Serial.println("[STORAGE] WARNING: failed to save timer");
  }
}

PersistedTimer storage_loadTimer(DeviceId id, TimerKind kind) {
  PersistedTimer t = {false, true, 0, 0};
  const char* key = timerKey(id, kind);
  if (prefs.isKey(key)) {
    prefs.getBytes(key, &t, sizeof(PersistedTimer));
  }
  return t;
}

void storage_saveLastEpoch(uint32_t epochSeconds) {
  if (prefs.putUInt(EPOCH_KEY, epochSeconds) == 0) {
    Serial.println("[STORAGE] WARNING: failed to save last-known time");
  }
}

uint32_t storage_loadLastEpoch() {
  return prefs.getUInt(EPOCH_KEY, 0);
}

void storage_saveApConfig(const ApConfig &cfg) {
  if (prefs.putBytes(AP_CONFIG_KEY, &cfg, sizeof(ApConfig)) != sizeof(ApConfig)) {
    Serial.println("[STORAGE] WARNING: failed to save AP config");
  }
}

ApConfig storage_loadApConfig() {
  ApConfig cfg;
  memset(&cfg, 0, sizeof(cfg));

  if (prefs.isKey(AP_CONFIG_KEY) &&
      prefs.getBytes(AP_CONFIG_KEY, &cfg, sizeof(ApConfig)) == sizeof(ApConfig)) {
    cfg.ssid[sizeof(cfg.ssid) - 1] = '\0';
    cfg.pass[sizeof(cfg.pass) - 1] = '\0';
    return cfg;
  }

  strncpy(cfg.ssid, WIFI_AP_NAME_DEFAULT, sizeof(cfg.ssid) - 1);
  strncpy(cfg.pass, WIFI_AP_PASS_DEFAULT, sizeof(cfg.pass) - 1);
  return cfg;
}

void storage_factoryReset() {
  prefs.clear();
}
