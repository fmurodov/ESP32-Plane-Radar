#include "hardware/display.h"

#include <Preferences.h>
#include <cstdlib>

#include "config.h"
#include "hardware/display_font.h"

LGFX tft;

namespace {

constexpr char kPrefsNamespace[] = "display";
constexpr char kPrefsBrightnessKey[] = "brightPct";

uint8_t s_brightness_percent = config::kDisplayDefaultBrightnessPercent;

uint8_t clampBrightnessPercent(int percent) {
  if (percent < config::kDisplayMinBrightnessPercent) {
    return config::kDisplayMinBrightnessPercent;
  }
  if (percent > config::kDisplayMaxBrightnessPercent) {
    return config::kDisplayMaxBrightnessPercent;
  }
  return static_cast<uint8_t>(percent);
}

void applyBrightness() {
  tft.setBrightness(static_cast<uint8_t>(
      (static_cast<uint16_t>(s_brightness_percent) * 255 + 50) / 100));
}

void loadBrightness() {
  Preferences prefs;
  if (!prefs.begin(kPrefsNamespace, true)) {
    return;
  }
  s_brightness_percent = clampBrightnessPercent(
      prefs.getUChar(kPrefsBrightnessKey, config::kDisplayDefaultBrightnessPercent));
  prefs.end();
}

void saveBrightness() {
  Preferences prefs;
  if (!prefs.begin(kPrefsNamespace, false)) {
    return;
  }
  prefs.putUChar(kPrefsBrightnessKey, s_brightness_percent);
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

void displaySaveBrightnessFromPortal(const char* value) {
  if (value == nullptr || value[0] == '\0') {
    return;
  }
  s_brightness_percent = clampBrightnessPercent(atoi(value));
  applyBrightness();
  saveBrightness();
  Serial.printf("Brightness: %u%%\n", s_brightness_percent);
}

void displayAdjustBrightness(int delta_percent) {
  s_brightness_percent = clampBrightnessPercent(
      static_cast<int>(s_brightness_percent) + delta_percent);
  applyBrightness();
  saveBrightness();
  Serial.printf("Brightness: %u%%\n", s_brightness_percent);
}

void displayResetBrightness() {
  s_brightness_percent = config::kDisplayDefaultBrightnessPercent;
  applyBrightness();
  Preferences prefs;
  if (prefs.begin(kPrefsNamespace, false)) {
    prefs.remove(kPrefsBrightnessKey);
    prefs.end();
  }
}
