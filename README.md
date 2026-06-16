# FansElectronics_DMD8266

[🇮🇩 Bahasa Indonesia](README-id.md)

---

## Description 📖
**FansElectronics_DMD8266** is a high-performance, flicker-free LED Dot Matrix library specifically designed for **ESP8266** microcontrollers.

This library empowers IoT developers and digital display manufacturers to:
- Drive **HUB08 and HUB12 (P10)** panels with ease
- Implement flicker-free animations using **Double Buffering**
- Control brightness efficiently via Output Enable (OE) pin management
- Create complex graphics with a robust coordinate system

If you want to use the ESP32 chip, use the library https://github.com/Vean/FansElectronics_DMD32

---

## What's New in v1.0.0 🚀

Initial release focused on **multi-panel stability and hardware flexibility** on the ESP8266 architecture.

### Why choose this library?
Unlike generic libraries, DMD8266 focuses on ESP8266 performance optimization:
- **Flexible Pin Mapping:** Configure custom GPIOs via a simple data structure (DmdPins).
- **Hardware SPI:** Utilizes native ESP8266 hardware SPI peripheral for high-speed data transfer (10MHz) to achieve maximum refresh rates.
- **Low-level Timer Interrupt:** Uses the ESP8266 internal hardware timer (os_timer_t) with a row scanning interval of ~100µs (~500Hz) to keep visuals rock-solid without blocking your main loop() function.
- **Memory Optimized:** Dynamic native heap allocation for efficient framebuffer management.

---

## Features ✨
- Unique **Double Buffer** system for zero-flicker visuals and smooth rendering transitions
- Native support for **HUB08 (P4.25, P7.62)** and **HUB12 (P10)** panels
- Adjustable brightness via specialized OE pin duty management
- Built-in text and font rendering engine with variable or fixed glyph width support
- Support for Daisy-chaining multiple panels horizontally and vertically
- Production-ready for 24/7 digital commercial displays

---

## Supported Panel Modes 🔒

| Panel Type | Scan Rate | Panel Model | Status |
|---|---|---|---|
| HUB08 | 1/16 Scan | P4.25 & P7.62 Single Color | ✅ Implemented |
| HUB12 | 1/4 Scan |  P10 Single Color | ✅ Implemented |

Perfect for:
- Commercial advertising displays
- Home Automation dashboards
- Running Text / Moving Message systems
- High-resolution LED matrix installations
- Digital Islamic Prayer Times (JWS) clocks

---

## How It Works ❓

1️⃣ **Initialize:** Define hardware control pins and panel configurations using the DmdPins struct.  
2️⃣ **Setup:** Call dmd.begin() to allocate matrix canvas memory and trigger automatic display row scanning interrupts.  
3️⃣ **Draw:** Use standard graphic primitive APIs (drawText, drawLine, drawRect, drawCircle) onto the framebuffer.  
4️⃣ **Refresh:** Periodically execute dmd.update() inside the loop() function to process SPI-based background row refreshes.

---

## Default Pins Configuration ⚙️

The DmdPins data structure allows you to flexibly map your matrix control pins on the ESP8266:
```cpp
// Example Pin Mapping for ESP8266 (e.g., NodeMCU)
DmdPins pinsDefault = {
  .pin_A     = 16,   // Multiplexing Row Address A (e.g., D0)
  .pin_B     = 5,    // Multiplexing Row Address B (e.g., D1)
  .pin_C     = 4,    // Row Address C (Set to -1 for HUB12 panels)
  .pin_D     = 0,    // Row Address D (Set to -1 for HUB12 panels)
  .pin_latch = 2,    // Latch / LAT / SCLK Pin (e.g., D4)
  .pin_sck   = 14,   // Hardware SPI Clock (D5 on NodeMCU)
  .pin_oe    = 15,   // Output Enable / OE Pin (e.g., D8)
  .pin_mosi  = 13    // Hardware SPI MOSI / DATA Pin (D7 on NodeMCU)
};

*Note: SCK and MOSI pins are internally bound to the ESP8266 native hardware SPI peripheral to guarantee high-speed bitstream throughput.*
```
---

## Quick Example 🔑

Here is a basic example showing how to display text using the FansElectronics_DMD8266 driver:
```cpp
#include <FansElectronics_DMD8266.h>
// Make sure to include a compatible font file
#include <fonts/SystemFont5x7.h> 

FansElectronics_DMD8266 dmd;

// Configure Pins according to your physical panel connections
DmdPins myPins = {16, 5, 4, 0, 2, 14, 15, 13};

void setup() {
    // Initialization: HUB08 panel type, 2 Panels Wide, 1 Panel High
    dmd.begin(PANEL_HUB08, 2, 1, myPins); 
    
    // Enable Double Buffer for smooth, flicker-free animations
    dmd.setDoubleBuffer(true);
    
    // Set display brightness (0 - 255)
    dmd.setBrightness(200);
    
    // Set active font
    // dmd.setFont(SystemFont5x7); 
}

void loop() {
    // Clear the background canvas buffer
    dmd.clear();
    
    // Draw text at coordinates X=2, Y=0
    dmd.drawText(2, 0, "FansElectronics");
    
    // Swap background drawing buffer onto the visual screen
    dmd.swapBuffers();
    
    // Handle background display refresh and timer updates
    dmd.update();
}
```
---

## Created By 💻
- **Writer** : Irfan Indra Kurniawan, ST
- **Organization** : Fans Electronics
- **Website** : [www.fanselectronics.com](https://www.fanselectronics.com)
- **Email** : info@fanselectronics.com

> **Author note:** Feel free to develop and utilize this library for educational, personal, religious, or commercial purposes.

---

## Buy me a Coffee ☕
- info@fanselectronics.com
- [www.facebook.com/FansElectronicsCom](https://www.facebook.com/FansElectronicsCom)
- [www.instagram.com/fanselectronics](https://www.instagram.com/fanselectronics)
- [www.saweria.co/FansElectronics](https://saweria.co/FansElectronics)

---

## Thanks To 🤲
- Allah Subhanahu Wa Ta'ala
- Arduino.cc
- GitHub
- Espressif Systems
- Contributors
- Everyone who has supported us and provided coffee ☕