#pragma once

#include <cstddef>

namespace services::adsb {

struct Aircraft {
  float lat;
  float lon;
  float nose_deg;
  float track_deg;
  float gs_knots;
  char callsign[9];
  char type[5];
  char alt[12];
};

constexpr size_t kMaxAircraft = 64;
constexpr size_t kLocalUrlMaxLen = 96;

size_t aircraftCount();
const Aircraft* aircraftList();

/** Hook invoked during long HTTP I/O (e.g. wifiLoop). Optional. */
using PollFn = void (*)();
void setPollFn(PollFn fn);

/** Fetch aircraft within fetch_radius_km of center_lat/lon: adsb.fi, or a local
 * receiver's aircraft.json if one is configured (see localUrl()). */
bool fetchUpdate(double center_lat, double center_lon, float fetch_radius_km);

/** Load saved local-receiver URL from NVS, or empty (use adsb.fi). Call once before WiFi setup. */
void sourceInit();
/** Empty = adsb.fi; non-empty = full URL to a local readsb/dump1090 aircraft.json. */
const char* localUrl();
/** WiFi portal text field: local receiver aircraft.json URL, or empty to use adsb.fi. */
void saveLocalUrlFromPortal(const char* url);
/** Clear the local URL override (e.g. with WiFi credential reset). */
void clearLocalUrl();

}  // namespace services::adsb
