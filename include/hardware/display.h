#pragma once

#include <cstdint>

#include "hardware/lgfx_config.hpp"

extern LGFX tft;

void displayInit();

/** Manual brightness, 1-100 (%). Used directly when auto day/night is off. */
uint8_t displayBrightnessPercent();
/** WiFi portal number field: "1"-"100"; applies immediately and saves to flash. */
void displaySaveBrightnessFromPortal(const char* value);
/** Reset brightness (manual + auto day/night settings) to defaults, e.g. alongside a WiFi reset. */
void displayResetBrightness();
/**
 * Step the active brightness by delta_percent (clamped) via the touch controls; applies and saves.
 * With auto day/night on, this nudges whichever level (day or night) is in effect right now.
 */
void displayAdjustBrightness(int delta_percent);

// --- Auto day/night brightness ---
/** True when brightness follows the day/night phase instead of the single manual level. */
bool displayAutoBrightnessEnabled();
/** Configured day-phase and night-phase brightness levels (%). */
uint8_t displayDayBrightnessPercent();
uint8_t displayNightBrightnessPercent();
/**
 * WiFi portal save: enable/disable auto brightness and set the day/night levels.
 * `enabled` is a checkbox value; `day`/`night` are "10"-"100" number-field strings.
 * Applies immediately and saves to flash.
 */
void displaySaveAutoBrightnessFromPortal(const char* enabled, const char* day, const char* night);
/**
 * Report the current day/night phase (true = daytime) from the clock. Re-applies brightness when
 * the phase flips and auto is on; a no-op otherwise. Called periodically from the main loop.
 */
void displaySetDaytime(bool is_daytime);
