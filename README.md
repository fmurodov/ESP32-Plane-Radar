# Plane Radar

<img width="800" height="450" alt="plane-radar" src="https://github.com/user-attachments/assets/716d0992-dab8-47ba-8f1a-2aec7f607419" />

**3D printed case (STL + assembly):** [MakerWorld](https://makerworld.com/en/models/2872376-esp32-plane-radar-live-ads-b-on-a-round-display#profileId-3207083) · **Firmware:** [Releases](https://github.com/fmurodov/ESP32-Plane-Radar/releases)

Firmware for a **1.28″ round GC9A01** display (240×240), on either an **ESP32-C3 Super Mini** (bare display module, manual wiring) or a **Seeed XIAO ESP32-C6** with the **Seeed "Round Display for XIAO"** (direct plug-in connector). Shows a circular **ADS-B radar** around your configured location, with **WiFiManager** for first-time setup.

## What it does

1. **Wi‑Fi setup** (if needed) — captive portal on AP **`PlaneRadar-Setup`**
2. **Radar** — live aircraft from [adsb.fi](https://opendata.adsb.fi/) on a sonar-style grid

After Wi‑Fi is saved, the device reconnects automatically; the radar runs in the main loop with periodic ADS-B updates (~5 s).

## Controls (BOOT, GPIO 9, active LOW)

| Action | Effect |
|--------|--------|
| **Short tap** | Cycle range preset (5 → 10 → 15 → 25 km); saved to flash |
| **Hold 3 s** | Clear Wi‑Fi, location, units, and brightness; reboot into setup portal |

During setup you can also hold BOOT at power-on to force a credential reset (same as the long press).

### Touch controls (Round Display for XIAO only)

The XIAO C6 build also reads the display's built-in CHSC6X touch panel; tap zones relative to
screen center (whichever axis has the bigger offset from center wins, so taps aren't ambiguous):

| Zone | Effect |
|------|--------|
| **Left** | Range preset: zoom out (wider area) |
| **Right** | Range preset: zoom in (tighter area) |
| **Top** | Brightness +10% |
| **Bottom** | Brightness −10% |

No touch hardware on the Super Mini build, so this is a no-op there. Zone boundaries haven't been
verified on real hardware yet — if left/right or top/bottom feel swapped or rotated, it's likely an
`offset_rotation` tweak needed on the touch config in `lgfx_config.hpp`.

## Wi‑Fi setup portal

**First-time setup** (no saved Wi‑Fi):

1. Connect to **`PlaneRadar-Setup`**
2. Open **`http://plane-radar.local`** (preferred) or **`http://192.168.4.1`** — both are shown on the yellow setup screen; captive portal may open automatically
3. Set home Wi‑Fi, then save

**Reconfigure anytime** (after the device is on your network):

1. Open **`http://plane-radar.local`** or **`http://<device-ip>`** (e.g. from your router or serial log at boot)
2. Change Wi‑Fi, location, units, or runway overlay; save

The same portal runs on the setup AP and on the device’s LAN IP while connected to Wi‑Fi. mDNS hostname is `plane-radar` → **plane-radar.local** (`kPortalHostname` in `config.h`). Some clients resolve `.local` slowly; use the IP if needed.

**Custom fields** (stored in NVS):

| Field | Purpose |
|-------|---------|
| **Latitude / Longitude** | Radar center and ADS-B query position (defaults in `config.h` until set) |
| **Display distances in miles** | Ring scale label in **mi** instead of **km** (e.g. `6mi` vs `10km`) |
| **Show airport runways** | Major-airport runway overlay on the radar (off to hide) |
| **Screen brightness (10-100%)** | Applies immediately on save; defaults to 100%. Handy for dimming at night. |
| **Local ADS-B receiver aircraft.json URL** | Blank (default) uses adsb.fi over the internet. Set to a local readsb/dump1090 `aircraft.json` URL (e.g. `http://192.168.1.50:8080/data/aircraft.json`) to use your own receiver instead — same per-aircraft JSON fields, so no other change needed. The firmware filters to the current range itself, since a local receiver returns everything it can hear (often far past this radar's max range), unlike adsb.fi which is filtered server-side. |

After a reset, the device reboots and shows the setup screen immediately (no “Connecting” loop on stale credentials).

## Radar display

### Grid

- Dark blue background, subdued green rings and crosshairs
- White **N / S / E / W** at the bezel; range label on the **east** spoke (ring 3 = ¾ of outer radius)
- White center dot

Layout and colors: `include/ui/radar_theme.h`.

### Range presets

| Ring 3 label | Outer radius (aircraft scale) |
|------------|-------------------------------|
| 5 km / 3 mi | ~6.7 km |
| 10 km / 6 mi | ~13.3 km (default) |
| 15 km / 9 mi | ~20 km |
| 25 km / 16 mi | ~33.3 km |

Preset and miles/km choice persist across reboot (`planeradar` NVS namespace).

### Runways

- Major airports from OurAirports (`large_airport`); all open runway strips in range (helipads excluded)
- Teal runway lines with one ICAO label per airport (e.g. `KJFK`); toggle in the Wi‑Fi setup portal
- Update the embedded list: `python3 scripts/build_large_airports.py`

### Aircraft

- **Inside the outer ring** — red heading triangle, magenta speed vector (clipped at the ring), callsign / type / altitude tags
- **Outside the ring** (still within ADS-B fetch) — small **red dot on the screen rim** at the correct bearing (direction cue; not distance-accurate past the ring)
- **Tags** — placed toward the **center**: west (left) → tag on the **right** of the symbol; east (right) → tag on the **left**

As range decreases (or aircraft approach), targets move inward; beyond-ring dots become full symbols when they cross the outer ring.

### ADS-B

- Source: [adsb.fi](https://opendata.adsb.fi/) by default, or a local readsb/dump1090 receiver if
  configured in the portal (see **Custom fields** above)
- Fetch radius: `ui::radar::fetchRadiusKm()` — scales with the active preset to roughly the screen edge (so rim dots have data)
- Poll interval: `kAdsbFetchIntervalMs` (5 s) in `config.h`
- Ground aircraft hidden by default (`kAdsbShowGroundAircraft`)

## Configuration

Edit **`include/config.h`** for hardware and behavior:

| Area | Keys / notes |
|------|----------------|
| Portal | `kPortalApName`, `kPortalIp`, `kPortalHostname` / `kPortalHostUrl` (mDNS; needs `-DWM_MDNS` in `platformio.ini`) |
| Wi‑Fi timing | connect attempts, reconnect grace, portal timeout (`0` = no timeout) |
| BOOT | `kBootPin`, `kBootResetHoldMs`, `kBootTapMinMs` |
| Display SPI | pins, `kDisplayInvert`, `kDisplayRgbOrder`, `kDisplaySpiWriteHz` |
| Default location | `kDefaultRadarLat`, `kDefaultRadarLon` (until portal overrides) |
| ADS-B | `kAdsbFetchIntervalMs`, `kAdsbShowGroundAircraft` |

Range presets: `include/ui/radar_range.h` (`kRangePresets`).

## Project layout

```
include/
  config.h
  hardware/
    lgfx_config.hpp
    display.h
    display_font.h
  data/
    large_airports.h
  ui/
    radar_theme.h
    radar_range.h
    radar_display.h
    runway_overlay.h
    status_screens.h
  services/
    wifi_setup.h
    radar_location.h
    adsb_client.h
data/
  ui_font.vlw              — embedded smooth UI font (Noto Sans Bold)
scripts/
  build_large_airports.py
src/
  main.cpp
  data/
    large_airports_data.cpp
  hardware/
  ui/
  services/
```

## Wiring

### GC9A01 ↔ ESP32-C3 Super Mini

| Display | ESP32-C3 |
|---------|----------|
| VCC | 3V3 |
| GND | GND |
| RST | GPIO **0** |
| CS | GPIO **1** |
| DC | GPIO **10** |
| SDA (MOSI) | GPIO **3** |
| SCL (SCLK) | GPIO **4** |
| BOOT (user) | GPIO **9** |

### Round Display for XIAO ↔ Seeed XIAO ESP32-C6

Plugs directly onto the XIAO via its onboard connector — no manual wiring. Pin mapping below is for
reference/troubleshooting only (from Seeed's `Seeed_Arduino_RoundDisplay` reference driver):

| Display signal | XIAO pin | ESP32-C6 GPIO |
|---|---|---|
| LCD CS | D1 | GPIO1 |
| LCD DC | D3 | GPIO21 |
| LCD RST | — | none (software reset only) |
| SCLK | D8 | GPIO19 |
| MOSI | D10 | GPIO18 |
| MISO | D9 | GPIO20 (unused) |
| Backlight enable | D6 | GPIO16 |
| BOOT (user) | — | GPIO9 (onboard XIAO button) |

> **Board switch:** the display has a 2-position slide switch (labeled **ON** / **KE**, near the
> microSD slot) with both slides needing to be set to their enabled side. Either slide left on the
> "digital" side disconnects D6 (backlight) and/or A0 (battery-voltage sense) and frees them as
> plain GPIO instead — if screen brightness doesn't respond to the portal setting at all (works in
> serial log, no visible change), check this switch first before suspecting firmware.

The display also carries a PCF8563 RTC on the same shared I2C bus (SDA = D4/GPIO22, SCL =
D5/GPIO23) — not currently used by this firmware. The CHSC6X touch controller (also on this bus,
INT = D7/GPIO17) *is* used — see [Touch controls](#touch-controls-round-display-for-xiao-only) above.

> Confirmed working on real hardware: display, WiFi, and brightness. Touch is new and not yet
> tested on hardware. If colors look inverted or swapped, adjust `kDisplayInvert` /
> `kDisplayRgbOrder` in `config.h`.

## Build

```bash
pio run -t upload -e supermini        # ESP32-C3 Super Mini + bare GC9A01
pio run -t upload -e xiao_c6_round    # XIAO ESP32-C6 + Round Display for XIAO
pio device monitor
```

- Serial: **115200** baud
- USB CDC on boot is enabled for both boards (native USB)
- `xiao_c6_round` uses the [pioarduino](https://github.com/pioarduino/platform-espressif32) platform
  fork, since the official PlatformIO `espressif32` platform has no Arduino-framework support for
  ESP32-C6 yet

### Web-flashable release image

Single `.bin` for [esptool-js](https://espressif.github.io/esptool-js/) and similar tools (4 MB flash, flash at **0x0**):

```bash
chmod +x scripts/merge-firmware.sh   # once
./scripts/merge-firmware.sh                       # defaults to supermini (ESP32-C3, 4 MB)
./scripts/merge-firmware.sh --env xiao_c6_round   # ESP32-C6, 4 MB
```

Writes `release/plane-radar-merged.bin`. Skip rebuild if firmware is already built:

```bash
./scripts/merge-firmware.sh --no-build
```

Or via PlatformIO only (output: `.pio/build/<env>/firmware-merged.bin`):

```bash
pio run -e supermini
pio run -t merge -e supermini
```

Put the board in download mode (hold **BOOT**, tap **RESET**), then flash with Chrome/Edge over USB.

### CI and releases (GitHub Actions)

| Workflow | When | Output |
|----------|------|--------|
| [Build](.github/workflows/build.yml) | Push / PR to `main` | Artifacts `plane-radar-supermini` and `plane-radar-xiao-c6-round` (merged + split `.bin` files, ~90 days) |
| [Release](.github/workflows/release.yml) | Git tag `v*` (e.g. `v1.0.0`) | GitHub Release assets `plane-radar-v1.0.0.bin` (Super Mini) and `plane-radar-v1.0.0-xiao-c6-round.bin`, each with a `.sha256` |

To ship a version users can download:

```bash
git tag v1.0.0
git push origin v1.0.0
```

The release workflow builds both firmwares in CI and attaches the merged images to the release.
Download the one matching your board from **Releases** on GitHub, then flash at **0x0** (4 MB flash).

## Dependencies

- [LovyanGFX](https://github.com/lovyan03/LovyanGFX)
- [WiFiManager](https://github.com/tzapu/WiFiManager)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)

---

This project is a fork of [MatixYo/ESP32-Plane-Radar](https://github.com/MatixYo/ESP32-Plane-Radar).
