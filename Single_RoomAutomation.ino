/*
  Single_RoomAutomation.ino
  Single-room ESP32 automation - Fan + LED, 2-relay standalone.

  Hardware:
    Fan relay  -> GPIO26 (active LOW)
    LED relay  -> GPIO27 (active LOW)
    Fan switch -> GPIO32 (maintained toggle, to GND, INPUT_PULLUP)
    LED switch -> GPIO33 (maintained toggle, to GND, INPUT_PULLUP)

  WiFi / setup flow:
    Boot tries to connect using the last-saved WiFi credentials (handled
    by the ESP32 core itself, no separate library needed for this part).
    If that doesn't succeed within STA_BOOT_CONNECT_TIMEOUT_MS, the device
    opens its own setup AP (name/password from Config.h defaults, or
    whatever was last saved via the dashboard's AP Config page) and starts
    a captive portal. Connecting to that AP and opening a browser goes
    straight to this same dashboard - there is no separate WiFiManager
    configuration menu. From the dashboard's Settings > WiFi Client page,
    scan for and connect to the home network; once connected the setup AP
    is dropped automatically.

  Required libraries (install via Arduino IDE Library Manager):
    ESPAsyncWebServer - use the actively maintained ESP32Async fork
      (https://github.com/ESP32Async/ESPAsyncWebServer), not the old
      unmaintained me-no-dev original.
    AsyncTCP - matching ESP32Async/AsyncTCP fork (same publisher as above)
    ArduinoJson - either v6.x or v7.x works, code auto-detects the
      installed version (see the ARDUINOJSON_VERSION_MAJOR check in
      WebDashboard.cpp)
    (DNSServer and WiFi are bundled with the ESP32 board package - no
    separate install needed)

  Board: ESP32 Dev Module. Compiles on Arduino-ESP32 core 2.x and 3.x.
*/

#include <WiFi.h>
#include <string.h>

#include "Config.h"
#include "StorageManager.h"
#include "TimeManager.h"
#include "DeviceControl.h"
#include "ScheduleManager.h"
#include "TimerManager.h"
#include "WebDashboard.h"

enum WifiPhase { PHASE_TRYING_STA, PHASE_AP_MODE, PHASE_STA_CONNECTED };
static WifiPhase phase = PHASE_TRYING_STA;
static unsigned long phaseStartMs = 0;

static unsigned long lastBlinkMs = 0;
static bool statusLedState = false;

static void statusLed_update() {
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    return;
  }
  unsigned long now = millis();
  if (now - lastBlinkMs > 500UL) {
    lastBlinkMs = now;
    statusLedState = !statusLedState;
    digitalWrite(STATUS_LED_PIN, statusLedState ? HIGH : LOW);
  }
}

static void enterApMode() {
  ApConfig apCfg = storage_loadApConfig();

  WiFi.mode(WIFI_AP_STA); // AP for setup, STA half stays alive in the background
  if (strlen(apCfg.pass) > 0) {
    WiFi.softAP(apCfg.ssid, apCfg.pass);
  } else {
    WiFi.softAP(apCfg.ssid);
  }

  webDashboard_startCaptivePortal();
  phase = PHASE_AP_MODE;
  Serial.println("[WIFI] No network yet - setup AP active: " + String(apCfg.ssid));
}

static void enterStaConnected() {
  phase = PHASE_STA_CONNECTED;
  webDashboard_stopCaptivePortal();
  WiFi.mode(WIFI_STA); // drop the setup AP now that a real network is available
  webDashboard_startMdns();
  Serial.println("[WIFI] Connected: " + WiFi.localIP().toString());
}

static unsigned long lastReconnectAttemptMs = 0;
static uint8_t consecutiveFailedAttempts = 0;

static void wifi_maintain() {
  if (phase == PHASE_TRYING_STA) {
    if (WiFi.status() == WL_CONNECTED) {
      enterStaConnected();
    } else if (millis() - phaseStartMs > STA_BOOT_CONNECT_TIMEOUT_MS) {
      enterApMode();
    }
    return;
  }

  if (phase == PHASE_AP_MODE) {
    if (WiFi.status() == WL_CONNECTED) {
      enterStaConnected();
    }
    return;
  }

  // PHASE_STA_CONNECTED - normal operation, watch for drops and retry.
  if (WiFi.status() == WL_CONNECTED) {
    consecutiveFailedAttempts = 0;
    return;
  }

  unsigned long now = millis();
  if (now - lastReconnectAttemptMs < WIFI_RECONNECT_CHECK_MS) return;
  lastReconnectAttemptMs = now;

  consecutiveFailedAttempts++;
  if (consecutiveFailedAttempts >= WIFI_RECONNECT_HARD_RETRY_AFTER) {
    consecutiveFailedAttempts = 0;
    WiFi.disconnect(false, false);
    WiFi.begin();
  } else {
    WiFi.reconnect();
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  storage_begin();
  deviceControl_begin();
  timeManager_begin();
  scheduleManager_begin();
  timerManager_begin();

  // Dashboard starts immediately and stays up regardless of WiFi mode -
  // it is reachable both during setup (as the captive portal page) and
  // during normal operation.
  webDashboard_begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(); // reuse whatever credentials were last saved, if any
  phaseStartMs = millis();
  phase = PHASE_TRYING_STA;
}

void loop() {
  deviceControl_loop();
  scheduleManager_loop();
  timerManager_loop();
  timeManager_loop();

  wifi_maintain();
  webDashboard_loop();

  statusLed_update();
}
