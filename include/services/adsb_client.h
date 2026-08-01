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
constexpr size_t kApiBaseMaxLen = 96;

size_t aircraftCount();
const Aircraft* aircraftList();

/** Hook invoked during long HTTP I/O (e.g. wifiLoop). Optional. */
using PollFn = void (*)();
void setPollFn(PollFn fn);

/** Fetch aircraft within fetch_radius_km of center_lat/lon, appending
 * "<lat>/lon/<lon>/dist/<dist_nm>" to the configured API base (see apiBase()). */
bool fetchUpdate(double center_lat, double center_lon, float fetch_radius_km);

/** Load saved API base URL from NVS, or empty (use the default, adsb.fi). Call once before
 * WiFi setup. */
void sourceInit();
/** Empty = default (adsb.fi); non-empty = base URL up to and including ".../lat/" for any
 * provider using the same "<base><lat>/lon/<lon>/dist/<dist_nm>" query shape and JSON schema
 * (e.g. a self-hosted adsb.fi/adsb.lol-API-compatible service). */
const char* apiBase();
/** WiFi portal text field: API base URL override, or empty to use the default. */
void saveApiBaseFromPortal(const char* url);
/** Clear the API base override (e.g. with WiFi credential reset). */
void clearApiBase();

}  // namespace services::adsb
