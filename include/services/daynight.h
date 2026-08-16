#pragma once

// Day/night clock: syncs UTC time over NTP (WiFi must be up) and computes the sun's elevation
// at a given location. Works entirely in UTC — sunrise/sunset fall out of the elevation, so no
// timezone or DST configuration is needed, just the radar's latitude/longitude.

namespace services {
namespace daynight {

/** Start SNTP sync. Idempotent; safe to call again after reconnects. Needs WiFi to actually sync. */
void init();

/** True once NTP has set the clock (elevation results are meaningless until then). */
bool timeValid();

/**
 * Sun elevation above the horizon in degrees at (lat, lon) right now, using NOAA's solar
 * position formulas. Positive = sun up. Returns a large negative sentinel if the time is not
 * yet valid. Compare against config::kDaylightElevationDeg to decide day vs. night.
 */
double solarElevationDeg(double lat, double lon);

}  // namespace daynight
}  // namespace services
