#pragma once

#include <cstdint>

#include <driver/gpio.h>

namespace config {

// --- Wi-Fi portal ---
constexpr char kPortalApName[] = "PlaneRadar-Setup";
constexpr char kPortalIp[] = "192.168.4.1";
/** mDNS host (no ".local" suffix); browser: http://plane-radar.local */
constexpr char kPortalHostname[] = "plane-radar";
constexpr char kPortalHostUrl[] = "plane-radar.local";

/** Per-attempt STA connect wait (ms); retried kWifiConnectAttempts times. */
constexpr unsigned long kWifiConnectAttemptMs = 15000;
constexpr uint8_t kWifiConnectAttempts = 3;
constexpr unsigned long kWifiPortalTimeoutSec = 0;  // 0 = no timeout while configuring
constexpr unsigned long kWifiConnectingFrameMs = 50;
/** Wait after disconnect before reconnecting (avoids portal on brief drops). */
constexpr unsigned long kWifiDownGraceMs = 4000;
/** Minimum interval between background reconnect tries. */
constexpr unsigned long kWifiReconnectIntervalMs = 15000;

// --- BOOT button (ESP32-C3 Super Mini, active LOW) ---
constexpr gpio_num_t kBootPin = GPIO_NUM_9;
constexpr unsigned long kBootResetHoldMs = 3000UL;
/** Ignore BOOT taps shorter than this (debounce). */
constexpr unsigned long kBootTapMinMs = 40UL;

// --- Display: GC9A01 1.28" round 240×240 (SPI) ---
#if defined(PLANE_RADAR_BOARD_XIAO_C6_ROUND)
// Seeed "Round Display for XIAO" plugged directly onto a XIAO ESP32-C6
// (pins per Seeed_Arduino_RoundDisplay reference driver, mapped to ESP32-C6 GPIOs)
constexpr gpio_num_t kDisplayPinRst = GPIO_NUM_NC;   // no dedicated line; GC9A01 software reset only
constexpr gpio_num_t kDisplayPinCs = GPIO_NUM_1;     // XIAO D1
constexpr gpio_num_t kDisplayPinDc = GPIO_NUM_21;    // XIAO D3
constexpr gpio_num_t kDisplayPinMosi = GPIO_NUM_18;  // XIAO D10
constexpr gpio_num_t kDisplayPinSclk = GPIO_NUM_19;  // XIAO D8
constexpr gpio_num_t kDisplayPinBl = GPIO_NUM_16;    // XIAO D6, backlight enable (active HIGH)
constexpr int kDisplayRotation = 3;  // -90° from the panel's default orientation
#else
// Bare GC9A01 module manually wired to an ESP32-C3 Super Mini
constexpr gpio_num_t kDisplayPinRst = GPIO_NUM_0;
constexpr gpio_num_t kDisplayPinCs = GPIO_NUM_1;
constexpr gpio_num_t kDisplayPinDc = GPIO_NUM_10;
constexpr gpio_num_t kDisplayPinMosi = GPIO_NUM_3;  // display SDA
constexpr gpio_num_t kDisplayPinSclk = GPIO_NUM_4;  // display SCL
constexpr gpio_num_t kDisplayPinBl = GPIO_NUM_NC;   // backlight tied directly to 3V3
constexpr int kDisplayRotation = 0;
#endif

constexpr int kDisplayWidth = 240;
constexpr int kDisplayHeight = 240;

constexpr uint32_t kDisplaySpiWriteHz = 40000000;
// GC9A01 modules often need invert + BGR for correct black/green output
constexpr bool kDisplayInvert = true;
constexpr bool kDisplayRgbOrder = true;

/** Default and allowed range for the WiFi-portal brightness setting (%). */
constexpr uint8_t kDisplayDefaultBrightnessPercent = 100;
constexpr uint8_t kDisplayMinBrightnessPercent = 10;
constexpr uint8_t kDisplayMaxBrightnessPercent = 100;

// --- Radar center defaults (overridden via WiFi setup portal) ---
constexpr double kDefaultRadarLat = 52.3676;
constexpr double kDefaultRadarLon = 4.9041;

/** Poll adsb.fi (API public limit: 1 req/s). */
constexpr unsigned long kAdsbFetchIntervalMs = 3000;
/** Legacy scale unused — fetch uses radar::fetchRadiusKm() to screen edge. */
constexpr float kAdsbFetchRadiusScale = 1.0f;
/** false = hide aircraft with alt_baro "ground"; true = show them too. */
constexpr bool kAdsbShowGroundAircraft = false;

// --- UI colors (RGB565) — status screens ---
constexpr uint16_t kColorBlack = 0x0000;
constexpr uint16_t kColorYellow = 0xFFE0;
constexpr uint16_t kTextOnYellow = kColorBlack;
constexpr uint16_t kTextOnBlack = 0xFFFF;

}  // namespace config
