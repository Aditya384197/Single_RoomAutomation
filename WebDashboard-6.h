#pragma once
/*
  WebDashboard.h
  ---------------
  ESPAsyncWebServer + WebSocket dashboard. Serves the HTML page, exposes
  a small REST API, and pushes live state to all connected browsers over
  WebSocket the instant anything changes (toggle, schedule fire, timer
  expiry, physical switch) - no polling needed on the client, though the
  page also polls every 5s as a safety net.

  The same server instance runs regardless of WiFi mode - in AP (setup)
  mode it doubles as a captive portal, serving this same dashboard for
  any request instead of a separate configuration menu. webDashboard_begin()
  should be called once at boot, before WiFi connects to anything.
*/

#include <Arduino.h>

void webDashboard_begin();     // call once at boot, regardless of WiFi mode
void webDashboard_loop();      // call every loop() - WS cleanup, captive DNS
void webDashboard_broadcastState(); // push current status JSON to all clients

// Call once, only after WiFi STA actually connects to a real network -
// mDNS (.local hostname) only makes sense on a real LAN, not in AP mode.
void webDashboard_startMdns();

// Captive portal DNS redirect - active only while the device is broadcasting
// its own setup AP, so any domain a phone tries to reach resolves to us
// instead, which is what makes the OS auto-open our dashboard as a
// "sign in to network" page rather than a generic WiFiManager-style menu.
void webDashboard_startCaptivePortal();
void webDashboard_stopCaptivePortal();
