#include "WebDashboard.h"
#include "Config.h"
#include "WebPage.h"
#include "DeviceControl.h"
#include "ScheduleManager.h"
#include "TimerManager.h"
#include "TimeManager.h"
#include "StorageManager.h"

#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#if ARDUINOJSON_VERSION_MAJOR >= 7
  typedef JsonDocument JsonDoc2048;
  typedef JsonDocument JsonDoc1024;
  typedef JsonDocument JsonDoc384;
#else
  typedef StaticJsonDocument<2048> JsonDoc2048;
  typedef StaticJsonDocument<1024> JsonDoc1024;
  typedef StaticJsonDocument<384> JsonDoc384;
#endif
#include <ESPmDNS.h>
#include <Update.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

static AsyncWebServer server(HTTP_PORT);
static AsyncWebSocket ws(WS_PATH);
static DNSServer dnsServer;
static bool captivePortalActive = false;

static void sendDashboardPage(AsyncWebServerRequest *request) {
  String page = FPSTR(DASHBOARD_HTML);
  page.replace("%API_KEY%", API_SECRET_KEY);
  request->send(200, "text/html", page);
}

static volatile bool otaInProgress = false;
static volatile unsigned long otaLastChunkMs = 0;
static int otaFailedAuth = 0;
static unsigned long otaLockoutUntil = 0;

static bool checkAuth(AsyncWebServerRequest *request) {
  if (!request->hasHeader("X-API-Key")) return false;
  return request->getHeader("X-API-Key")->value().equals(API_SECRET_KEY);
}

static JsonDoc2048 statusDoc;
static SemaphoreHandle_t statusMutex = NULL;

static String buildStatusJson() {
  if (statusMutex == NULL || xSemaphoreTake(statusMutex, pdMS_TO_TICKS(250)) != pdTRUE) {
    return String("{\"error\":\"busy\"}");
  }

  statusDoc.clear();
  JsonDoc2048 &doc = statusDoc;

  doc["time"]      = timeManager_getTimeString();
  doc["date"]      = timeManager_getDateString();
  doc["synced"]    = timeManager_isSynced();
  doc["ntpSynced"] = timeManager_isNtpSynced();
  doc["wifi"]      = (WiFi.status() == WL_CONNECTED);

  JsonObject devices = doc["devices"].to<JsonObject>();
  const DeviceId ids[DEV_COUNT] = { DEV_FAN, DEV_LED };

  for (int i = 0; i < DEV_COUNT; i++) {
    DeviceId id = ids[i];
    JsonObject d = devices[deviceName(id)].to<JsonObject>();
    d["state"]  = deviceControl_getState(id);
    d["source"] = deviceControl_getLastSource(id);

    PersistedSchedule s = scheduleManager_get(id);
    JsonObject sc = d["schedule"].to<JsonObject>();
    sc["enabled"] = s.enabled;
    sc["onHour"]  = s.onHour;
    sc["onMin"]   = s.onMin;
    sc["offHour"] = s.offHour;
    sc["offMin"]  = s.offMin;

    JsonObject autoOn = d["autoOn"].to<JsonObject>();
    autoOn["active"]       = timerManager_isActive(id, TIMER_AUTO_ON);
    autoOn["remainingSec"] = timerManager_remainingSeconds(id, TIMER_AUTO_ON);
    autoOn["totalSec"]     = timerManager_totalSeconds(id, TIMER_AUTO_ON);

    JsonObject autoOff = d["autoOff"].to<JsonObject>();
    autoOff["active"]       = timerManager_isActive(id, TIMER_AUTO_OFF);
    autoOff["remainingSec"] = timerManager_remainingSeconds(id, TIMER_AUTO_OFF);
    autoOff["totalSec"]     = timerManager_totalSeconds(id, TIMER_AUTO_OFF);
  }

  String out;
  serializeJson(doc, out);
  xSemaphoreGive(statusMutex);
  return out;
}

void webDashboard_broadcastState() {
  if (ws.count() > 0) {
    ws.textAll(buildStatusJson());
  }
}

static bool parseDeviceId(const char* name, DeviceId &out) {
  if (!name) return false;
  if (strcmp(name, "fan") == 0) { out = DEV_FAN; return true; }
  if (strcmp(name, "led") == 0) { out = DEV_LED; return true; }
  return false;
}

static bool parseTimerKind(const char* name, TimerKind &out) {
  if (!name) return false;
  if (strcmp(name, "on") == 0)  { out = TIMER_AUTO_ON;  return true; }
  if (strcmp(name, "off") == 0) { out = TIMER_AUTO_OFF; return true; }
  return false;
}

#define BODY_BUF_SIZE 384
static char bodyBuf[BODY_BUF_SIZE];
static size_t bodyLen = 0;
static bool bodyOverflowed = false;

typedef void (*JsonBodyHandler)(JsonDocument &doc, AsyncWebServerRequest *request);

static void onJsonBody(AsyncWebServerRequest *request, uint8_t *data, size_t len,
                        size_t index, size_t total, JsonBodyHandler handler) {
  if (index == 0) {
    bodyLen = 0;
    bodyOverflowed = false;
  }

  if (!bodyOverflowed) {
    if (bodyLen + len <= BODY_BUF_SIZE) {
      memcpy(bodyBuf + bodyLen, data, len);
      bodyLen += len;
    } else {
      bodyOverflowed = true;
    }
  }

  if (index + len == total) {
    if (bodyOverflowed) {
      request->send(413, "application/json", "{\"ok\":false,\"error\":\"body_too_large\"}");
    } else if (!checkAuth(request)) {
      request->send(401, "application/json", "{\"ok\":false,\"error\":\"unauthorized\"}");
    } else {
      JsonDoc384 doc;
      DeserializationError err = deserializeJson(doc, bodyBuf, bodyLen);
      if (err) {
        request->send(400, "application/json", "{\"ok\":false,\"error\":\"bad_json\"}");
      } else {
        handler(doc, request);
      }
    }
  }
}

static void handleToggle(JsonDocument &doc, AsyncWebServerRequest *request) {
  DeviceId id;
  if (!parseDeviceId(doc["device"] | "", id)) {
    request->send(400, "application/json", "{\"ok\":false,\"error\":\"bad_device\"}");
    return;
  }
  bool state = doc["state"] | false;
  deviceControl_setState(id, state, "dashboard");
  request->send(200, "application/json", "{\"ok\":true}");
}

static void handleSchedule(JsonDocument &doc, AsyncWebServerRequest *request) {
  DeviceId id;
  if (!parseDeviceId(doc["device"] | "", id)) {
    request->send(400, "application/json", "{\"ok\":false,\"error\":\"bad_device\"}");
    return;
  }
  bool enabled     = doc["enabled"] | false;
  uint8_t onHour   = doc["onHour"]  | 0;
  uint8_t onMin    = doc["onMin"]   | 0;
  uint8_t offHour  = doc["offHour"] | 0;
  uint8_t offMin   = doc["offMin"]  | 0;

  scheduleManager_set(id, enabled, onHour, onMin, offHour, offMin);
  webDashboard_broadcastState();
  request->send(200, "application/json", "{\"ok\":true}");
}

static void handleTimerStart(JsonDocument &doc, AsyncWebServerRequest *request) {
  DeviceId id;
  TimerKind kind;
  if (!parseDeviceId(doc["device"] | "", id) || !parseTimerKind(doc["kind"] | "", kind)) {
    request->send(400, "application/json", "{\"ok\":false,\"error\":\"bad_params\"}");
    return;
  }
  uint32_t minutes = doc["minutes"] | 1;
  timerManager_start(id, kind, minutes);
  request->send(200, "application/json", "{\"ok\":true}");
}

static void handleTimerCancel(JsonDocument &doc, AsyncWebServerRequest *request) {
  DeviceId id;
  TimerKind kind;
  if (!parseDeviceId(doc["device"] | "", id) || !parseTimerKind(doc["kind"] | "", kind)) {
    request->send(400, "application/json", "{\"ok\":false,\"error\":\"bad_params\"}");
    return;
  }
  timerManager_cancel(id, kind);
  request->send(200, "application/json", "{\"ok\":true}");
}

static void handleApConfig(JsonDocument &doc, AsyncWebServerRequest *request) {
  const char* ssid = doc["ssid"] | "";
  const char* pass = doc["pass"] | "";
  if (strlen(ssid) == 0) {
    request->send(400, "application/json", "{\"ok\":false,\"error\":\"ssid_required\"}");
    return;
  }
  if (strlen(pass) > 0 && strlen(pass) < 8) {
    request->send(400, "application/json", "{\"ok\":false,\"error\":\"password_too_short\"}");
    return;
  }
  ApConfig cfg;
  memset(&cfg, 0, sizeof(cfg));
  strncpy(cfg.ssid, ssid, sizeof(cfg.ssid) - 1);
  strncpy(cfg.pass, pass, sizeof(cfg.pass) - 1);
  storage_saveApConfig(cfg);
  request->send(200, "application/json", "{\"ok\":true}");
  delay(300);
  ESP.restart();
}

static void handleFactoryReset(JsonDocument &doc, AsyncWebServerRequest *request) {
  (void)doc;
  storage_factoryReset();
  request->send(200, "application/json", "{\"ok\":true}");
  delay(300);
  WiFi.disconnect(true, true);
  delay(200);
  ESP.restart();
}

static void handleWifiScan(AsyncWebServerRequest *request) {
  if (!checkAuth(request)) {
    request->send(401, "application/json", "{\"ok\":false,\"error\":\"unauthorized\"}");
    return;
  }

  int n = WiFi.scanComplete();

  if (n == WIFI_SCAN_RUNNING) {
    request->send(200, "application/json", "{\"status\":\"scanning\"}");
    return;
  }

  if (n >= 0) {
    JsonDoc1024 doc;
    doc["status"] = "done";
    JsonArray arr = doc["networks"].to<JsonArray>();
    for (int i = 0; i < n; i++) {
      JsonObject net = arr.add<JsonObject>();
      net["ssid"] = WiFi.SSID(i);
      net["rssi"] = WiFi.RSSI(i);
    }
    WiFi.scanDelete();
    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
    return;
  }

  // WIFI_SCAN_FAILED or no scan started yet - kick off a new async scan.
  WiFi.scanNetworks(true);
  request->send(200, "application/json", "{\"status\":\"scanning\"}");
}

static bool wifiConnectPending = false;
static unsigned long wifiConnectStartMs = 0;
static String wifiConnectTargetSsid = "";

static void handleWifiConnect(JsonDocument &doc, AsyncWebServerRequest *request) {
  const char* ssid = doc["ssid"] | "";
  const char* pass = doc["pass"] | "";
  if (strlen(ssid) == 0) {
    request->send(400, "application/json", "{\"ok\":false,\"error\":\"ssid_required\"}");
    return;
  }

  wifiConnectTargetSsid = String(ssid);
  wifiConnectPending = true;
  wifiConnectStartMs = millis();
  WiFi.begin(ssid, pass);

  request->send(200, "application/json", "{\"ok\":true,\"status\":\"connecting\"}");
}

static void handleWifiStatus(AsyncWebServerRequest *request) {
  if (!checkAuth(request)) {
    request->send(401, "application/json", "{\"ok\":false,\"error\":\"unauthorized\"}");
    return;
  }

  bool actuallyConnected = (WiFi.status() == WL_CONNECTED) &&
                            (!wifiConnectPending || WiFi.SSID() == wifiConnectTargetSsid);

  if (actuallyConnected) {
    wifiConnectPending = false;
    request->send(200, "application/json",
                  "{\"status\":\"connected\",\"ip\":\"" + WiFi.localIP().toString() + "\"}");
    return;
  }

  if (wifiConnectPending) {
    if (millis() - wifiConnectStartMs > WIFI_CLIENT_CONNECT_TIMEOUT_MS) {
      wifiConnectPending = false;
      request->send(200, "application/json", "{\"status\":\"failed\"}");
    } else {
      request->send(200, "application/json", "{\"status\":\"connecting\"}");
    }
    return;
  }

  request->send(200, "application/json", "{\"status\":\"idle\"}");
}

static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                       AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    client->text(buildStatusJson());
  }
}

static bool otaCheckAuth(AsyncWebServerRequest *request, unsigned long now) {
  if (now < otaLockoutUntil) return false;
  if (!request->authenticate(OTA_AUTH_USER, OTA_AUTH_PASS)) {
    otaFailedAuth++;
    if (otaFailedAuth >= OTA_MAX_FAILED_AUTH) {
      otaLockoutUntil = now + OTA_LOCKOUT_MS;
      otaFailedAuth = 0;
    }
    return false;
  }
  otaFailedAuth = 0;
  return true;
}

// Manually build the 401 challenge instead of calling the library's
// requestAuthentication() helper - that method's signature has changed
// across ESPAsyncWebServer forks/versions and is a common source of
// compile errors. A plain response with a WWW-Authenticate header does
// the same job (triggers the browser's Basic Auth login prompt) using
// only the stable, version-independent response API.
static void sendOtaAuthChallenge(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse(401, "text/plain", "Authentication required");
  response->addHeader("WWW-Authenticate", "Basic realm=\"OTA Update\"");
  request->send(response);
}

static void handleOtaComplete(AsyncWebServerRequest *request) {
  unsigned long now = millis();
  if (now < otaLockoutUntil) {
    request->send(429, "text/plain", "Locked out, try again later");
    return;
  }
  if (!otaCheckAuth(request, now)) {
    sendOtaAuthChallenge(request);
    return;
  }
  bool ok = !Update.hasError();
  AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", ok ? "OK" : "FAIL");
  response->addHeader("Connection", "close");
  request->send(response);
  otaInProgress = false;
  if (ok) {
    delay(500);
    ESP.restart();
  }
}

static void handleOtaUpload(AsyncWebServerRequest *request, String filename, size_t index,
                             uint8_t *data, size_t len, bool final) {
  unsigned long now = millis();
  if (now < otaLockoutUntil) {
    request->send(429, "text/plain", "Locked out");
    return;
  }
  if (!otaCheckAuth(request, now)) {
    sendOtaAuthChallenge(request);
    return;
  }

  if (index == 0) {
    if (otaInProgress) {
      Update.abort();
    }
    otaInProgress = true;
    otaLastChunkMs = now;

    if (len > 0 && data[0] != 0xE9) {
      otaInProgress = false;
      request->send(400, "text/plain", "Invalid firmware image");
      return;
    }
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      otaInProgress = false;
      request->send(500, "text/plain", "Update.begin failed");
      return;
    }
  }

  if (!otaInProgress || !Update.isRunning()) return;

  otaLastChunkMs = now;
  if (Update.write(data, len) != len) {
    Update.abort();
    otaInProgress = false;
    request->send(500, "text/plain", "Write failed");
    return;
  }

  if (final) {
    if (!Update.end(true)) {
      otaInProgress = false;
      request->send(500, "text/plain", "Verification failed");
    }
  }
}

void webDashboard_begin() {
  statusMutex = xSemaphoreCreateMutex();

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, sendDashboardPage);

  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", buildStatusJson());
  });

  server.on("/api/toggle", HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    nullptr,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      onJsonBody(request, data, len, index, total, handleToggle);
    });

  server.on("/api/schedule", HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    nullptr,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      onJsonBody(request, data, len, index, total, handleSchedule);
    });

  server.on("/api/timer/start", HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    nullptr,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      onJsonBody(request, data, len, index, total, handleTimerStart);
    });

  server.on("/api/timer/cancel", HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    nullptr,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      onJsonBody(request, data, len, index, total, handleTimerCancel);
    });

  server.on("/api/apconfig", HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    nullptr,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      onJsonBody(request, data, len, index, total, handleApConfig);
    });

  server.on("/api/factoryreset", HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    nullptr,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      onJsonBody(request, data, len, index, total, handleFactoryReset);
    });

  server.on("/api/wifiscan", HTTP_GET, handleWifiScan);

  server.on("/api/wifi/status", HTTP_GET, handleWifiStatus);

  server.on("/api/wifi/connect", HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    nullptr,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      onJsonBody(request, data, len, index, total, handleWifiConnect);
    });

  server.on("/update", HTTP_POST, handleOtaComplete, handleOtaUpload);

  // While the setup AP + captive portal is active, ANY unmatched request
  // (including the various OS captive-portal probe URLs like
  // /generate_204, /hotspot-detect.html, /connecttest.txt) gets this same
  // dashboard instead of a 404 - that mismatch between what the OS expects
  // and what it actually gets is exactly what makes it auto-open this page
  // as a "sign in to network" prompt. Once fully configured (no captive
  // portal running), unmatched requests just get a plain 404 as normal.
  server.onNotFound([](AsyncWebServerRequest *request) {
    if (captivePortalActive && request->method() == HTTP_GET) {
      sendDashboardPage(request);
      return;
    }
    request->send(404, "application/json", "{\"ok\":false,\"error\":\"not_found\"}");
  });

  server.begin();
}

void webDashboard_startMdns() {
  if (MDNS.begin(MDNS_HOSTNAME)) {
    MDNS.addService("http", "tcp", HTTP_PORT);
  }
}

void webDashboard_startCaptivePortal() {
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  captivePortalActive = true;
}

void webDashboard_stopCaptivePortal() {
  dnsServer.stop();
  captivePortalActive = false;
}

void webDashboard_loop() {
  if (captivePortalActive) {
    dnsServer.processNextRequest();
  }

  static unsigned long lastCleanup = 0;
  unsigned long now = millis();
  if (now - lastCleanup > 2000UL) {
    lastCleanup = now;
    ws.cleanupClients();
  }

  if (otaInProgress && (now - otaLastChunkMs > OTA_STALL_TIMEOUT_MS)) {
    Update.abort();
    otaInProgress = false;
  }
}
