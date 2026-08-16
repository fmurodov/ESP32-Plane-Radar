#include "services/daynight.h"

#include <Arduino.h>

#include <cmath>
#include <ctime>

#include "config.h"

namespace services {
namespace daynight {

namespace {

// Unix time reads pre-2023 until NTP has run — the gate for "is the clock set yet?".
constexpr time_t kSyncedThreshold = 1700000000;  // 2023-11-14

}  // namespace

void init() { configTime(0, 0, config::kNtpServer); }

bool timeValid() { return time(nullptr) >= kSyncedThreshold; }

double solarElevationDeg(double lat, double lon) {
  const time_t now = time(nullptr);
  if (now < kSyncedThreshold) {
    return -90.0;  // unknown time — caller treats this as "before sunrise"
  }

  struct tm utc;
  gmtime_r(&now, &utc);
  const double minutes = utc.tm_hour * 60.0 + utc.tm_min + utc.tm_sec / 60.0;

  // NOAA solar position (see the NOAA Solar Calculator). All angles in degrees unless noted.
  const double jd = static_cast<double>(now) / 86400.0 + 2440587.5;
  const double jc = (jd - 2451545.0) / 36525.0;  // Julian centuries since J2000.0

  double l0 = fmod(280.46646 + jc * (36000.76983 + jc * 0.0003032), 360.0);
  if (l0 < 0.0) {
    l0 += 360.0;
  }
  const double m = 357.52911 + jc * (35999.05029 - 0.0001537 * jc);
  const double e = 0.016708634 - jc * (0.000042037 + 0.0000001267 * jc);
  const double m_rad = radians(m);
  const double eqctr = sin(m_rad) * (1.914602 - jc * (0.004817 + 0.000014 * jc)) +
                       sin(2 * m_rad) * (0.019993 - 0.000101 * jc) +
                       sin(3 * m_rad) * 0.000289;
  const double true_long = l0 + eqctr;
  const double omega = 125.04 - 1934.136 * jc;
  const double lambda = true_long - 0.00569 - 0.00478 * sin(radians(omega));
  const double eps0 =
      23.0 + (26.0 + (21.448 - jc * (46.815 + jc * (0.00059 - jc * 0.001813))) / 60.0) / 60.0;
  const double eps = eps0 + 0.00256 * cos(radians(omega));
  const double decl = degrees(asin(sin(radians(eps)) * sin(radians(lambda))));

  double y = tan(radians(eps / 2.0));
  y *= y;
  const double eqtime =
      4.0 * degrees(y * sin(2 * radians(l0)) - 2 * e * sin(m_rad) +
                    4 * e * y * sin(m_rad) * cos(2 * radians(l0)) -
                    0.5 * y * y * sin(4 * radians(l0)) - 1.25 * e * e * sin(2 * m_rad));

  double true_solar = fmod(minutes + eqtime + 4.0 * lon, 1440.0);
  if (true_solar < 0.0) {
    true_solar += 1440.0;
  }
  double hour_angle = true_solar / 4.0;
  hour_angle += (hour_angle < 0.0) ? 180.0 : -180.0;

  const double lat_rad = radians(lat);
  const double decl_rad = radians(decl);
  double cos_zenith = sin(lat_rad) * sin(decl_rad) +
                      cos(lat_rad) * cos(decl_rad) * cos(radians(hour_angle));
  cos_zenith = constrain(cos_zenith, -1.0, 1.0);
  return 90.0 - degrees(acos(cos_zenith));
}

}  // namespace daynight
}  // namespace services
