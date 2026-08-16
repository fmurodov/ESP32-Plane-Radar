/**
 * Plane Radar — WiFi setup, then radar UI on the round GC9A01 display.
 */

#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "hardware/display.h"
#include "services/adsb_client.h"
#include "services/daynight.h"
#include "services/radar_location.h"
#include "services/wifi_setup.h"
#include "ui/radar_display.h"
#include "ui/radar_range.h"
#include "ui/status_screens.h"

namespace {

bool g_radar_visible = false;
unsigned long g_wifi_down_since = 0;
unsigned long g_last_reconnect_ms = 0;
unsigned long g_last_adsb_fetch_ms = 0;
unsigned long g_last_daynight_ms = 0;

/** Re-evaluate day vs. night from the clock and let the display adjust auto brightness. */
void updateDayNight() {
  if (g_last_daynight_ms != 0 &&
      millis() - g_last_daynight_ms < config::kDaynightPollMs) {
    return;
  }
  g_last_daynight_ms = millis();
  if (!services::daynight::timeValid()) {
    return;
  }
  const double elevation = services::daynight::solarElevationDeg(
      services::location::lat(), services::location::lon());
  displaySetDaytime(elevation > config::kDaylightElevationDeg);
}

void showRadarIfConnected() {
  if (WiFi.status() != WL_CONNECTED) {
    g_radar_visible = false;
    return;
  }
  ui::radarDisplayDraw();
  g_radar_visible = true;
}

void onRangeChanged() {
  char range_label[12];
  char outer_label[12];
  ui::radar::formatCurrentRing3Label(range_label, sizeof(range_label));
  ui::radar::formatRing3Label(outer_label, sizeof(outer_label),
                               ui::radar::rangeCurrent().outer_km, ui::radar::useNm());
  Serial.printf("Range: %s (outer ~%s)\n", range_label, outer_label);

  if (g_radar_visible && WiFi.status() == WL_CONNECTED) {
    ui::radarDisplayDraw();
  }
}

void onRangeTap() {
  ui::radar::rangeNext();
  onRangeChanged();
}

void handleBootButton() {
  bootButtonPollLongPress();
  if (bootButtonConsumeTap()) {
    onRangeTap();
  }
}

/**
 * Touch zones on the round display (Round Display for XIAO only — getTouch() is a
 * harmless no-op on boards without touch hardware): left/right of center steps the range
 * preset (zoom out/in), top/bottom adjusts brightness. Whichever axis has the larger
 * offset from center wins, so a tap is never ambiguous between the two.
 */
void handleTouch() {
  static bool s_touch_active = false;
  int32_t x = 0;
  int32_t y = 0;
  const bool touched = tft.getTouch(&x, &y) > 0;
  if (!touched) {
    s_touch_active = false;
    return;
  }
  if (s_touch_active) {
    return;
  }
  s_touch_active = true;

  const int32_t dx = x - config::kDisplayWidth / 2;
  const int32_t dy = y - config::kDisplayHeight / 2;
  if (abs(dx) > abs(dy)) {
    if (dx > 0) {
      ui::radar::rangeNext();
    } else {
      ui::radar::rangePrev();
    }
    onRangeChanged();
  } else {
    displayAdjustBrightness(dy < 0 ? config::kTouchBrightnessStepPercent
                                    : -config::kTouchBrightnessStepPercent);
  }
}

void fetchAndDrawAircraft() {
  const float fetch_km = ui::radar::fetchRadiusKm();
  if (!services::adsb::fetchUpdate(services::location::lat(),
                                   services::location::lon(), fetch_km)) {
    handleBootButton();
    return;
  }
  ui::radarDisplayRefreshAircraft();
  handleBootButton();
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("Plane Radar");

  bootButtonInit();
  displayInit();
  if (wifiShowsSetupScreenOnBoot()) {
    statusScreenPortal();
  }
  services::location::init();
  ui::radar::rangeInit();
  services::adsb::sourceInit();
  services::adsb::setPollFn(wifiLoop);
  services::daynight::init();

  if (wifiSetupConnect()) {
    showRadarIfConnected();
  }
}

void loop() {
  handleBootButton();
  handleTouch();
  wifiLoop();
  updateDayNight();

  if (WiFi.status() != WL_CONNECTED) {
    if (g_radar_visible) {
      Serial.println("WiFi lost — will reconnect");
      g_radar_visible = false;
    }

    if (g_wifi_down_since == 0) {
      g_wifi_down_since = millis();
    }

    const unsigned long down_ms = millis() - g_wifi_down_since;
    if (down_ms >= config::kWifiDownGraceMs &&
        millis() - g_last_reconnect_ms >= config::kWifiReconnectIntervalMs) {
      g_last_reconnect_ms = millis();
      if (wifiReconnect()) {
        g_wifi_down_since = 0;
        showRadarIfConnected();
      }
    }
  } else {
    g_wifi_down_since = 0;
    if (!g_radar_visible) {
      showRadarIfConnected();
    } else if (millis() - g_last_adsb_fetch_ms >= config::kAdsbFetchIntervalMs) {
      g_last_adsb_fetch_ms = millis();
      fetchAndDrawAircraft();
    }
  }

  delay(10);
}
