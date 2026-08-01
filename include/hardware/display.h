#pragma once

#include <cstdint>

#include "hardware/lgfx_config.hpp"

extern LGFX tft;

void displayInit();

/** Current brightness, 1-100 (%). */
uint8_t displayBrightnessPercent();
/** WiFi portal number field: "1"-"100"; applies immediately and saves to flash. */
void displaySaveBrightnessFromPortal(const char* value);
/** Reset brightness to the default (e.g. alongside a full WiFi/units reset). */
void displayResetBrightness();
/** Step brightness by delta_percent (clamped, e.g. via touch controls); applies and saves. */
void displayAdjustBrightness(int delta_percent);
