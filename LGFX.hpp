#include "pins_arduino.h"
#include <LovyanGFX.hpp>
#include <SPI.h>

// Create a class that inherits from LGFX_Device and defines your configuration.
class LGFX : public lgfx::LGFX_Device
{
private:
  // Declare the panel and bus instances
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI     _bus_instance;

public:
  // Constructor
  LGFX(void) {
    // --- Bus configuration (SPI) ---
    auto bus_cfg = _bus_instance.config();
    bus_cfg.spi_mode = 0;             // SPI mode 0
    bus_cfg.freq_write = 40000000;    // 40MHz write frequency
    bus_cfg.freq_read = 16000000;     // 16MHz read frequency
    bus_cfg.spi_3wire = true;         // Use 3-wire SPI (MOSI, SCLK, CS; MISO not needed)
    bus_cfg.pin_sclk = D5;            // SPI SCLK pin (e.g., D5 on NodeMCU)
    bus_cfg.pin_mosi = D7;            // SPI MOSI pin (e.g., D7 on NodeMCU)
    bus_cfg.pin_miso = -1;            // MISO pin (set to -1 if not used, common for displays)
    bus_cfg.pin_dc   = D3;            // Data/Command pin (e.g., D6 on NodeMCU)
    _bus_instance.config(bus_cfg);    // Pass the configuration to the bus
    _panel_instance.setBus(&_bus_instance); // Set the bus on the panel

    // --- Panel configuration (ST7789) ---
    auto panel_cfg = _panel_instance.config();
    panel_cfg.pin_cs = D8; // Chip Select pin (e.g., D8 on NodeMCU)
    panel_cfg.pin_rst = -1; // Reset pin (e.g., D3 on NodeMCU, or connect to RST for auto-reset)
    panel_cfg.pin_busy = -1; // Busy pin (set to -1 if not used)
    panel_cfg.panel_width = 240; // Display width
    panel_cfg.panel_height = 240; // Display height (adjust for your specific display e.g. 240x240, 135x240)
    panel_cfg.memory_width = 240;
    panel_cfg.memory_height = 240;
    panel_cfg.offset_x = 0; // X offset
    panel_cfg.offset_y = 0; // Y offset
    panel_cfg.invert = true; // Invert colors (true or false depending on display)
    _panel_instance.config(panel_cfg); // Pass the configuration to the panel

    // --- Backlight configuration (optional) ---
    // If you have a backlight pin, uncomment and configure the following:
    // lgfx::Light_PWM _light_instance;
    // auto light_cfg = _light_instance.config();
    // light_cfg.pin_bl = 4; // Backlight pin (e.g., D2 on NodeMCU)
    // light_cfg.invert = false;
    // _light_instance.config(light_cfg);
    // _panel_instance.setLight(&_light_instance);

    setPanel(&_panel_instance); // Set the panel instance as the active panel
  }
};
