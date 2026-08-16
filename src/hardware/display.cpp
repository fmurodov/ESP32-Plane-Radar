#include "hardware/display.h"

#include <Preferences.h>
#include <cstdlib>

#include "config.h"
#include "hardware/display_font.h"

LGFX tft;

namespace {

constexpr char kPrefsNamespace[] = "display";
constexpr char kPrefsBrightnessKey[] = "brightPct";
constexpr char kPrefsAutoKey[] = "autoBri";
constexpr char kPrefsDayKey[] = "dayPct";
constexpr char kPrefsNightKey[] = "nightPct";

uint8_t s_brightness_percent = config::kDisplayDefaultBrightnessPercent;
bool s_auto_enabled = config::kDisplayDefaultAutoBrightness;
uint8_t s_day_percent = config::kDisplayDefaultDayBrightnessPercent;
uint8_t s_night_percent = config::kDisplayDefaultNightBrightnessPercent;
bool s_is_daytime = true;  // until the clock reports otherwise

uint8_t clampBrightnessPercent(int percent) {
  if (percent < config::kDisplayMinBrightnessPercent) {
    return config::kDisplayMinBrightnessPercent;
  }
  if (percent > config::kDisplayMaxBrightnessPercent) {
    return config::kDisplayMaxBrightnessPercent;
  }
  return static_cast<uint8_t>(percent);
}

// Level currently in effect: the day/night level when auto is on, else the manual one. Returned by
// reference so the touch controls can nudge whichever one is live.
uint8_t& activeBrightness() {
  if (!s_auto_enabled) {
    return s_brightness_percent;
  }
  return s_is_daytime ? s_day_percent : s_night_percent;
}

void applyBrightness() {
  tft.setBrightness(static_cast<uint8_t>(
      (static_cast<uint16_t>(activeBrightness()) * 255 + 50) / 100));
}

void loadBrightness() {
  Preferences prefs;
  if (!prefs.begin(kPrefsNamespace, true)) {
    return;
  }
  s_brightness_percent = clampBrightnessPercent(
      prefs.getUChar(kPrefsBrightnessKey, config::kDisplayDefaultBrightnessPercent));
  s_auto_enabled = prefs.getBool(kPrefsAutoKey, config::kDisplayDefaultAutoBrightness);
  s_day_percent = clampBrightnessPercent(
      prefs.getUChar(kPrefsDayKey, config::kDisplayDefaultDayBrightnessPercent));
  s_night_percent = clampBrightnessPercent(
      prefs.getUChar(kPrefsNightKey, config::kDisplayDefaultNightBrightnessPercent));
  prefs.end();
}

void saveBrightness() {
  Preferences prefs;
  if (!prefs.begin(kPrefsNamespace, false)) {
    return;
  }
  prefs.putUChar(kPrefsBrightnessKey, s_brightness_percent);
  prefs.putBool(kPrefsAutoKey, s_auto_enabled);
  prefs.putUChar(kPrefsDayKey, s_day_percent);
  prefs.putUChar(kPrefsNightKey, s_night_percent);
  prefs.end();
}

}  // namespace

void displayInit() {
  loadBrightness();
  tft.init();
  tft.setRotation(config::kDisplayRotation);
  applyBrightness();
  tft.setTextWrap(false);
  displayFontInit();
}

uint8_t displayBrightnessPercent() { return s_brightness_percent; }
bool displayAutoBrightnessEnabled() { return s_auto_enabled; }
uint8_t displayDayBrightnessPercent() { return s_day_percent; }
uint8_t displayNightBrightnessPercent() { return s_night_percent; }

void displaySaveBrightnessFromPortal(const char* value) {
  if (value == nullptr || value[0] == '\0') {
    return;
  }
  s_brightness_percent = clampBrightnessPercent(atoi(value));
  applyBrightness();
  saveBrightness();
  Serial.printf("Brightness (manual): %u%%\n", s_brightness_percent);
}

void displaySaveAutoBrightnessFromPortal(const char* enabled, const char* day,
                                         const char* night) {
  s_auto_enabled = enabled != nullptr && enabled[0] != '\0' &&
                   enabled[0] != 'F' && enabled[0] != 'f';
  if (day != nullptr && day[0] != '\0') {
    s_day_percent = clampBrightnessPercent(atoi(day));
  }
  if (night != nullptr && night[0] != '\0') {
    s_night_percent = clampBrightnessPercent(atoi(night));
  }
  applyBrightness();
  saveBrightness();
  Serial.printf("Auto brightness: %s (day %u%%, night %u%%)\n",
                s_auto_enabled ? "on" : "off", s_day_percent, s_night_percent);
}

void displayAdjustBrightness(int delta_percent) {
  uint8_t& level = activeBrightness();
  level = clampBrightnessPercent(static_cast<int>(level) + delta_percent);
  applyBrightness();
  saveBrightness();
  if (s_auto_enabled) {
    Serial.printf("Brightness (%s): %u%%\n", s_is_daytime ? "day" : "night", level);
  } else {
    Serial.printf("Brightness: %u%%\n", level);
  }
}

void displaySetDaytime(bool is_daytime) {
  if (is_daytime == s_is_daytime) {
    return;
  }
  s_is_daytime = is_daytime;
  if (s_auto_enabled) {
    applyBrightness();
    Serial.printf("Auto brightness: %s → %u%%\n", is_daytime ? "day" : "night",
                  activeBrightness());
  }
}

void displayResetBrightness() {
  s_brightness_percent = config::kDisplayDefaultBrightnessPercent;
  s_auto_enabled = config::kDisplayDefaultAutoBrightness;
  s_day_percent = config::kDisplayDefaultDayBrightnessPercent;
  s_night_percent = config::kDisplayDefaultNightBrightnessPercent;
  applyBrightness();
  Preferences prefs;
  if (prefs.begin(kPrefsNamespace, false)) {
    prefs.remove(kPrefsBrightnessKey);
    prefs.remove(kPrefsAutoKey);
    prefs.remove(kPrefsDayKey);
    prefs.remove(kPrefsNightKey);
    prefs.end();
  }
}
