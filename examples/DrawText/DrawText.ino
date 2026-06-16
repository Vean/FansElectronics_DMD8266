/*
 * Creator             : Irfan Indra Kurniawan, ST
 * Website             : https://www.fanselectronics.com
 * Buy Me a Coffee     : https://saweria.co/FansElectronics
 * Tested on Wemos D1 mini ESP8266-12E with HUB08 and HUB12 Panels
 */

// ID: Tambahkan Library
// EN: Add Library
#include <FansElectronics_DMD8266.h>

// ID: Tambahkan Font Anda
// EN: Add Your Font
#include <fonts/System6x7.h>
#include <fonts/SystemFont5x7.h>

// ID: Instansiasi Objek Utama Library
// EN: Main Library Object Instantiation
FansElectronics_DMD8266 dmd;

// =========================================================================
// 1. ID: KONFIGURASI PIN UNTUK TIPE HUB08 (P4.75 / P7.62 / Videotron)
//    EN: PIN CONFIGURATION FOR HUB08 PANEL TYPE (P4.75 / P7.62 / Videotron)
// =========================================================================
DmdPins pinsHUB08 = {
    .pin_A = 16,    // D0 (GPIO16)
    .pin_B = 12,    // D6 (GPIO12)
    .pin_C = 2,     // D4 (GPIO2)
    .pin_D = 3,     // RX (GPIO3)
    .pin_latch = 0, // D3 (GPIO0) -> STB / LATCH
    .pin_sck = 14,  // D5 (GPIO14) -> CLK SPI Hardware
    .pin_oe = 15,   // D8 (GPIO15) -> nOE
    .pin_mosi = 13  // D7 (GPIO13) -> R1 / MOSI SPI Hardware
};
// =========================================================================
// 2. ID: KONFIGURASI PIN UNTUK TIPE HUB12 (P10 Single/Multi-Panel)
//    EN: PIN CONFIGURATION FOR HUB12 PANEL TYPE (P10 Single/Multi-Panel)
// =========================================================================
DmdPins pinsHUB12 = {
    .pin_A = 16,    // D0 (GPIO16)
    .pin_B = 12,    // D6 (GPIO12)
    .pin_C = -1,    // ID: HUB12 tidak menggunakan pin C
    .pin_D = -1,    // ID: HUB12 tidak menggunakan pin D
    .pin_latch = 0, // D3 (GPIO0) -> LATCH
    .pin_sck = 14,  // D5 (GPIO14) -> SCK SPI Hardware
    .pin_oe = 15,   // D8 (GPIO15) -> nOE
    .pin_mosi = 13  // D7 (GPIO13) -> DATA / MOSI SPI Hardware
};

void setup()
{
    // -------------------------------------------------------------------------
    // ID: PANDUAN PENGUJIAN: Buka salah satu baris .begin() di bawah ini
    // EN: TESTING GUIDE: Uncomment one of the .begin() lines below
    // -------------------------------------------------------------------------

    // ID : Jika menggunakan 2 Panel HUB08 Tunggal (Resolusi 128 x 16 piksel)
    // EN : If using 2 Single HUB08 Panels (Resolution 128 x 16 pixels)
    dmd.begin(PANEL_HUB08, 2, 1, pinsHUB08);

    // ID : Jika menggunakan 2 Panel HUB12 (P10) Jajar Horizontal (Resolusi 64 x 16 piksel)
    // EN : If using 2 HUB12 Panels (P10) Horizontal Array (Resolution
    // dmd.begin(PANEL_HUB12, 2, 1, pinsHUB12);

    // -------------------------------------------------------------------------
    // ID: Mengatur tingkat kecerahan LED (0 = Mati, 255 = Maksimal)
    // EN: Set LED brightness level (0 = Off, 255 = Maximum)
    dmd.setBrightness(128);
}

void loop()
{
    // 1. ID: Atur font pertama dan cetak teks di baris atas
    //    EN: Set first font and draw text on the top row
    dmd.setFont(System6x7);
    dmd.drawText(12, -1, "FansElectronics"); // ID: Koordinat X=2, Y=0 | EN: X=2, Y=0 Coordinates

    // 2. ID: Atur font kedua dan cetak teks di baris bawah
    //    EN: Set second font and draw text on the bottom row
    dmd.setFont(SystemFont5x7);
    dmd.drawText(12, 9, "DMD8266 MULTI-PANEL"); // ID: Koordinat X=2, Y=8 | EN: X=2, Y=8 Coordinates

    // 3. ID: Trigger fungsi interupsi untuk memindai fisik baris LED via SPI
    //    EN: Trigger interrupt function to scan physical LED rows via SPI
    dmd.update();
}