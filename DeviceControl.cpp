#include "DeviceControl.h"
#include "StorageManager.h"
#include "TimerManager.h"
#include <string.h>

// Forward declaration only (defined in WebDashboard.cpp) - avoids a heavy
// #include of ESPAsyncWebServer inside this file.
void webDashboard_broadcastState();

struct DeviceRuntime {
  uint8_t relayPin;
  uint8_t switchPin;

  bool relayState;
  char lastSource[10];

  // debounce state machine (non-blocking, millis based)
  int lastRawReading;
  int stableReading;
  unsigned long lastEdgeMs;

  // delayed/coalesced flash write - protects NVS write-cycle lifetime
  // against someone flicking the switch rapidly many times in a row.
  bool lastSavedState;      // what's currently committed to flash
  bool saveDirty;           // relayState differs from lastSavedState, write pending
  unsigned long dirtySinceMs;
};

static DeviceRuntime dev[DEV_COUNT];

static void applyRelay(DeviceId id) {
  digitalWrite(dev[id].relayPin, dev[id].relayState ? RELAY_ON : RELAY_OFF);
}

void deviceControl_begin() {
  const uint8_t relayPins[DEV_COUNT]  = { FAN_RELAY_PIN, LED_RELAY_PIN };
  const uint8_t switchPins[DEV_COUNT] = { FAN_SWITCH_PIN, LED_SWITCH_PIN };

  for (int i = 0; i < DEV_COUNT; i++) {
    dev[i].relayPin  = relayPins[i];
    dev[i].switchPin = switchPins[i];
    strncpy(dev[i].lastSource, "boot", sizeof(dev[i].lastSource) - 1);
    dev[i].lastSource[sizeof(dev[i].lastSource) - 1] = '\0';

    pinMode(dev[i].relayPin, OUTPUT);
    // Set OFF first so relay never glitches ON for a moment at boot
    digitalWrite(dev[i].relayPin, RELAY_OFF);

    pinMode(dev[i].switchPin, INPUT_PULLUP);

    // Restore last known state from flash (power-loss recovery)
    bool savedState = storage_loadDeviceState((DeviceId)i);
    dev[i].relayState = savedState;
    dev[i].lastSavedState = savedState;
    dev[i].saveDirty = false;
    applyRelay((DeviceId)i);

    // Prime debounce state with current switch reading so that the very
    // first loop() does NOT think a physical toggle just happened.
    int r = digitalRead(dev[i].switchPin);
    dev[i].lastRawReading = r;
    dev[i].stableReading  = r;
    dev[i].lastEdgeMs = millis();
  }
}

void deviceControl_setState(DeviceId id, bool on, const char* source) {
  strncpy(dev[id].lastSource, source, sizeof(dev[id].lastSource) - 1);
  dev[id].lastSource[sizeof(dev[id].lastSource) - 1] = '\0';

  bool isHumanAction = (strcmp(source, "manual") == 0) || (strcmp(source, "dashboard") == 0);
  if (isHumanAction) {
    timerManager_cancelAllForDevice(id);
  }

  if (dev[id].relayState == on) {
    return;
  }
  dev[id].relayState = on;

  applyRelay(id);

  // Don't write flash immediately - mark dirty and let deviceControl_loop()
  // commit it once the state has been stable for STATE_SAVE_DELAY_MS. This
  // coalesces rapid toggles (e.g. someone flicking the switch several times)
  // into a single flash write instead of one per toggle.
  dev[id].saveDirty = true;
  dev[id].dirtySinceMs = millis();

  webDashboard_broadcastState();
}

bool deviceControl_getState(DeviceId id) {
  return dev[id].relayState;
}

const char* deviceControl_getLastSource(DeviceId id) {
  return dev[id].lastSource;
}

static void checkSwitch(DeviceId id) {
  DeviceRuntime &d = dev[id];
  int reading = digitalRead(d.switchPin);

  if (reading != d.lastRawReading) {
    // raw pin flickered - restart the debounce timer
    d.lastRawReading = reading;
    d.lastEdgeMs = millis();
    return;
  }

  if ((millis() - d.lastEdgeMs) < DEBOUNCE_MS) {
    return; // still settling
  }

  if (reading != d.stableReading) {
    // Confirmed, debounced physical switch position change -> toggle relay.
    d.stableReading = reading;
    deviceControl_setState(id, !d.relayState, "manual");
  }
}

static void commitPendingSave(DeviceId id) {
  DeviceRuntime &d = dev[id];
  if (!d.saveDirty) return;
  if (millis() - d.dirtySinceMs < STATE_SAVE_DELAY_MS) return;

  // Only actually touch flash if the state is still different from what's
  // already saved (it might have flipped back and forth and landed on the
  // same value it started at - no need to write in that case).
  if (d.relayState != d.lastSavedState) {
    storage_saveDeviceState(id, d.relayState);
    d.lastSavedState = d.relayState;
  }
  d.saveDirty = false;
}

void deviceControl_loop() {
  checkSwitch(DEV_FAN);
  checkSwitch(DEV_LED);
  commitPendingSave(DEV_FAN);
  commitPendingSave(DEV_LED);
}
