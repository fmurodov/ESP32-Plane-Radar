#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#if defined(PLANE_RADAR_BOARD_XIAO_C6_ROUND)
#include <driver/i2c.h>
#endif

#include "config.h"

/** LovyanGFX device: GC9A01 on SPI. Pin values come from config.h. */
class LGFX : public lgfx::LGFX_Device {
  lgfx::Bus_SPI _bus;
  lgfx::Panel_GC9A01 _panel;
  lgfx::Light_PWM _light;
#if defined(PLANE_RADAR_BOARD_XIAO_C6_ROUND)
  lgfx::Touch_CHSC6X _touch;
#endif

public:
  LGFX() {
    {
      auto cfg = _bus.config();
      cfg.spi_host = SPI2_HOST;
      cfg.freq_write = config::kDisplaySpiWriteHz;
      cfg.pin_sclk = static_cast<int>(config::kDisplayPinSclk);
      cfg.pin_mosi = static_cast<int>(config::kDisplayPinMosi);
      cfg.pin_miso = -1;
      cfg.pin_dc = static_cast<int>(config::kDisplayPinDc);
      _bus.config(cfg);
      _panel.setBus(&_bus);
    }
    {
      auto cfg = _panel.config();
      cfg.pin_cs = static_cast<int>(config::kDisplayPinCs);
      cfg.pin_rst = static_cast<int>(config::kDisplayPinRst);
      cfg.invert = config::kDisplayInvert;
      cfg.rgb_order = config::kDisplayRgbOrder;
      _panel.config(cfg);
    }
    // Boards with a GPIO-gated backlight (e.g. Round Display for XIAO); bare
    // GC9A01 modules with BL tied straight to 3V3 leave this pin unset (-1).
    if constexpr (config::kDisplayPinBl != GPIO_NUM_NC) {
      auto cfg = _light.config();
      cfg.pin_bl = static_cast<int16_t>(config::kDisplayPinBl);
      _light.config(cfg);
      _panel.setLight(&_light);
    }
#if defined(PLANE_RADAR_BOARD_XIAO_C6_ROUND)
    {
      auto cfg = _touch.config();
      cfg.i2c_port = I2C_NUM_0;
      cfg.pin_sda = static_cast<int>(config::kTouchPinSda);
      cfg.pin_scl = static_cast<int>(config::kTouchPinScl);
      cfg.pin_int = static_cast<int>(config::kTouchPinInt);
      cfg.x_min = 0;
      cfg.x_max = config::kDisplayWidth - 1;
      cfg.y_min = 0;
      cfg.y_max = config::kDisplayHeight - 1;
      _touch.config(cfg);
      _panel.setTouch(&_touch);
    }
#endif
    setPanel(&_panel);
  }
};
