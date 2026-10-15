//----------------------------------------------------------------------------------------------
/*
 * Creator             : Irfan Indra Kurniawan, ST
 * Website             : https://www.fanselectronics.com
 * Buy Me a Coffee     : https://saweria.co/FansElectronics
 * Ported for Multi-Panel Type (HUB08 & HUB12) on ESP8266
 * Support P10, P4.25, P7.62, dan panel custom dengan konfigurasi pin fleksibel
 */

#include "FansElectronics_DMD8266.h"
#include "Arduino.h"
#include "string.h"
#include "stdlib.h"
#include "FansElectronics_Bitmap.h"
extern "C"
{
#include "user_interface.h"
#include "osapi.h"
#define USE_US_TIMER
#define os_timer_arm_us(a, b, c) ets_timer_arm_new(a, b, c, 0)
}
os_timer_t dispTimer;
bool tickOccured;

#define DMD_REFRESH_US 100 // Refresh rate ~500Hz (2000µs per refresh)

// ID: Tabel pembalik urutan bit (MSB ke LSB) untuk pemindaian baris zigzag HUB12
// EN: Bit reversal lookup table (MSB to LSB) for HUB12 zigzag row scanning
static const uint8_t flipBits[256] PROGMEM = {
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0,
    0x30, 0xB0, 0x70, 0xF0, 0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
    0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8, 0x04, 0x84, 0x44, 0xC4,
    0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC,
    0x3C, 0xBC, 0x7C, 0xFC, 0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
    0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2, 0x0A, 0x8A, 0x4A, 0xCA,
    0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6,
    0x36, 0xB6, 0x76, 0xF6, 0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
    0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE, 0x01, 0x81, 0x41, 0xC1,
    0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9,
    0x39, 0xB9, 0x79, 0xF9, 0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
    0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5, 0x0D, 0x8D, 0x4D, 0xCD,
    0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3,
    0x33, 0xB3, 0x73, 0xF3, 0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
    0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB, 0x07, 0x87, 0x47, 0xC7,
    0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF,
    0x3F, 0xBF, 0x7F, 0xFF};

//----------------------------------------------------------------------------------------------
FansElectronics_DMD8266::FansElectronics_DMD8266()
    : FansElectronics_Bitmap(0, 0), _doubleBuffer(false), phase(0), fb0(0), fb1(0), displayfb(0),
      lastRefresh(millis()), _scanMode(SCAN_ZIGZAG), _maskD0(0), _maskD1(0), _maskD2(0), _maskCLK(0), _maskUnusedData(0)
{
    brightenss = 255;
    fb0 = displayfb = fb;
}

//----------------------------------------------------------------------------------------------
FansElectronics_DMD8266::~FansElectronics_DMD8266()
{
    // ID: Pembersihan memori alokasi dinamis untuk mencegah memori bocor
    // EN: Deallocate dynamic memory regions to avoid serious memory leaks
    if (fb0)
        free(fb0);
    if (fb1)
        free(fb1);
    fb = 0;
}
//----------------------------------------------------------------------------------------------
void timerCallback(void *pArg)
{
    tickOccured = true;
}
//----------------------------------------------------------------------------------------------
void dispinit(void)
{
    system_timer_reinit();
    os_timer_setfn(&dispTimer, timerCallback, NULL);
    os_timer_arm_us(&dispTimer, DMD_REFRESH_US, true);
}
//----------------------------------------------------------------------------------------------
void FansElectronics_DMD8266::begin(PanelType type, int panelsWide, int panelsHigh, DmdPins pins, int panelWidthPx, int panelHeightPx, ScanMode scanMode)
{
    _panelType = type;
    _panelsWide = (panelsWide < 1) ? 1 : panelsWide;
    _panelsHigh = (panelsHigh < 1) ? 1 : panelsHigh;
    _pins = pins;
    _scanMode = scanMode;

    // ID: Pembatasan jumlah baris maksimal 3 untuk mode SCAN_PARALLEL pada HUB12 di ESP8266
    // EN: Limit maximum panel rows to 3 when using SCAN_PARALLEL on HUB12 for ESP8266
    if (_panelType == PANEL_HUB12 && _scanMode == SCAN_PARALLEL)
    {
        if (_panelsHigh > 3)
        {
            _panelsHigh = 3;
        }
    }
    else
    {
        // HUB08 selalu menggunakan mode standar
        _scanMode = SCAN_ZIGZAG;
    }

    // ID: Pengaman deteksi resolusi otomatis jika masukan pengguna bernilai nol
    // EN: Auto-resolution fallback mechanism if user parameters are zero
    if (panelWidthPx == 0 || panelHeightPx == 0)
    {
        if (_panelType == PANEL_HUB08)
        {
            _panelWidthPx = 64; // Standar HUB08 (P4.25 / P7.62)
            _panelHeightPx = 16;
        }
        else
        {
            _panelWidthPx = 32; // Standar HUB12 (P10)
            _panelHeightPx = 16;
        }
    }
    else
    {
        _panelWidthPx = panelWidthPx;
        _panelHeightPx = panelHeightPx;
    }

    // ID: Menghitung total dimensi kanvas horizontal dan vertikal
    // EN: Computing global horizontal and vertical canvas boundaries
    _width = _panelsWide * _panelWidthPx;
    _height = _panelsHigh * _panelHeightPx;
    _stride = (_width + 7) / 8;

    // ID: Alokasi heap memori untuk buffer frame visual primer
    // EN: Dynamic heap allocation for the primary video framebuffer
    if (fb0)
        free(fb0);
    unsigned int size = _stride * _height;
    fb0 = (uint8_t *)malloc(size);
    if (fb0)
    {
        memset(fb0, 0xFF, size);
    }
    fb = displayfb = fb0;

    // ID: Konfigurasi arah kendali GPIO register matriks
    // EN: Configuration of matrix shift-register GPIO control flow
    pinMode(_pins.pin_A, OUTPUT);
    pinMode(_pins.pin_B, OUTPUT);
    if (_pins.pin_C != -1)
        pinMode(_pins.pin_C, OUTPUT);
    if (_pins.pin_D != -1)
        pinMode(_pins.pin_D, OUTPUT);
    pinMode(_pins.pin_latch, OUTPUT);
    pinMode(_pins.pin_oe, OUTPUT);

    digitalWrite(_pins.pin_A, LOW);
    digitalWrite(_pins.pin_B, LOW);
    if (_pins.pin_C != -1)
        digitalWrite(_pins.pin_C, LOW);
    if (_pins.pin_D != -1)
        digitalWrite(_pins.pin_D, LOW);
    digitalWrite(_pins.pin_latch, LOW);
    digitalWrite(_pins.pin_oe, (_panelType == PANEL_HUB08) ? HIGH : LOW);

    // ID: Hitung bitmask GPIO untuk jalur data dan clock
    // EN: Precompute GPIO bitmasks for data and clock lines
    _maskD0 = (_pins.pin_data1 >= 0 && _pins.pin_data1 < 16) ? (1UL << _pins.pin_data1) : 0;
    _maskD1 = (_pins.pin_data2 >= 0 && _pins.pin_data2 < 16) ? (1UL << _pins.pin_data2) : 0;
    _maskD2 = (_pins.pin_data3 >= 0 && _pins.pin_data3 < 16) ? (1UL << _pins.pin_data3) : 0;
    _maskCLK = (_pins.pin_sck >= 0 && _pins.pin_sck < 16) ? (1UL << _pins.pin_sck) : 0;
    _maskUnusedData = 0;

    if (_panelType == PANEL_HUB12)
    {
        // ID: Pengaman Anti-Silau: Set semua pin data tambahan yang terhubung ke HIGH (Padam)
        // EN: Anti-Glare Safety: Pull any connected auxiliary data pins HIGH (Blanked)
        if (_pins.pin_data2 != -1)
        {
            pinMode(_pins.pin_data2, OUTPUT);
            digitalWrite(_pins.pin_data2, HIGH); // HIGH = Mati pada HUB12
        }
        if (_pins.pin_data3 != -1)
        {
            pinMode(_pins.pin_data3, OUTPUT);
            digitalWrite(_pins.pin_data3, HIGH); // HIGH = Mati pada HUB12
        }

        // Tentukan konektor mana yang tidak terpakai berdasarkan jumlah baris (_panelsHigh)
        if (_scanMode == SCAN_ZIGZAG || _panelsHigh == 1)
        {
            _maskUnusedData = _maskD1 | _maskD2; // Baris 2 & 3 dipaksa mati
        }
        else if (_panelsHigh == 2)
        {
            _maskUnusedData = _maskD2; // Baris 3 dipaksa mati
        }
    }

    if (_panelType == PANEL_HUB12 && _scanMode == SCAN_PARALLEL)
    {
        // ID: Mode Multi-Konektor (Parallel) menggunakan GPIO Output murni (Fast Bit-Banging)
        // EN: Multi-Connector (Parallel) mode uses direct GPIO Output (Fast Bit-Banging)
        pinMode(_pins.pin_sck, OUTPUT);
        pinMode(_pins.pin_data1, OUTPUT);
        digitalWrite(_pins.pin_sck, LOW);
        digitalWrite(_pins.pin_data1, HIGH);
    }
    else
    {
        // ID: Mode Zigzag / Standar menggunakan Hardware SPI
        // EN: Zigzag / Standard mode uses Hardware SPI
        pinMode(SCK, SPECIAL);
        pinMode(MOSI, SPECIAL);
        SPI1C = 0;
        SPI1U = SPIUMOSI | SPIUDUPLEX | SPIUSSE;
        SPI1U1 = (7 << SPILMOSI) | (7 << SPILMISO);
        SPI1C1 = 0;
        SPI1C &= ~(SPICWBO | SPICRBO);
        SPI1U &= ~(SPIUSME);
        SPI1P &= ~(1 << 29);
        SPI.setFrequency(10000000);
    }

    tickOccured = false;
    dispinit();
}

//----------------------------------------------------------------------------------------------
// ID: Bit-Banging 1 Baris (Sekaligus memastikan konektor Baris 2 & 3 tetap HIGH / Padam)
// EN: 1-Row Bit-Banging (Simultaneously keeps Row 2 & 3 connectors HIGH / Blanked)
inline void FansElectronics_DMD8266::shiftOutParallel1(uint8_t b0, uint32_t maskD0, uint32_t maskCLK, uint32_t maskUnused)
{
    for (uint8_t bit = 0x80; bit != 0; bit >>= 1)
    {
        if (b0 & bit)
            GPOS = maskD0 | maskUnused;
        else
        {
            if (maskUnused)
                GPOS = maskUnused;
            GPOC = maskD0;
        }
        GPOS = maskCLK;
        GPOC = maskCLK;
    }
}

//----------------------------------------------------------------------------------------------
// ID: Bit-Banging Cepat untuk 2 Baris Panel HUB12 (Konektor 1: pin_data1, Konektor 2: pin_data2)
// EN: Fast Bit-Banging Routine for 2 Rows of HUB12 Panels (Conn 1: pin_data1, Conn 2: pin_data2)
inline void FansElectronics_DMD8266::shiftOutParallel2(uint8_t b0, uint8_t b1, uint32_t maskD0, uint32_t maskD1, uint32_t maskCLK, uint32_t maskAllActive, uint32_t maskUnused)
{
    for (uint8_t bit = 0x80; bit != 0; bit >>= 1)
    {
        uint32_t setBits = maskUnused; // Pastikan konektor ke-3 (jika ada) tetap HIGH (Mati)
        if (b0 & bit)
            setBits |= maskD0;
        if (b1 & bit)
            setBits |= maskD1;

        GPOS = setBits;                                     // Set pin yang bernilai HIGH
        GPOC = (maskAllActive ^ (setBits & maskAllActive)); // Clear pin yang bernilai LOW
        GPOS = maskCLK;                                     // Clock HIGH
        GPOC = maskCLK;                                     // Clock LOW
    }
}

//----------------------------------------------------------------------------------------------
// ID: Bit-Banging Cepat untuk 3 Baris Panel HUB12 (Konektor 1: pin_data1, Konektor 2: pin_data2, Konektor 3: pin_data3)
// EN: Fast Bit-Banging Routine for 3 Rows of HUB12 Panels (Conn 1: pin_data1, Conn 2: pin_data2, Conn 3: pin_data3)
inline void FansElectronics_DMD8266::shiftOutParallel3(uint8_t b0, uint8_t b1, uint8_t b2, uint32_t maskD0, uint32_t maskD1, uint32_t maskD2, uint32_t maskCLK, uint32_t maskAllActive)
{
    for (uint8_t bit = 0x80; bit != 0; bit >>= 1)
    {
        uint32_t setBits = 0;
        if (b0 & bit)
            setBits |= maskD0;
        if (b1 & bit)
            setBits |= maskD1;
        if (b2 & bit)
            setBits |= maskD2;

        GPOS = setBits;                   // Set pin yang bernilai HIGH
        GPOC = (maskAllActive ^ setBits); // Clear pin yang bernilai LOW
        GPOS = maskCLK;                   // Clock HIGH
        GPOC = maskCLK;                   // Clock LOW
    }
}

//----------------------------------------------------------------------------------------------
void FansElectronics_DMD8266::update()
{
    if (tickOccured == true)
    {
        tickOccured = false;
        refresh();
    }
}

//----------------------------------------------------------------------------------------------
void FansElectronics_DMD8266::refresh()
{
    if (_panelType == PANEL_HUB08)
    {
        // 1. BLANKING: Matikan OE sebelum kirim data SPI
        digitalWrite(_pins.pin_oe, HIGH);

        int stride16 = _stride * 16;
        volatile uint8_t *data0;

        for (int y = 0; y < _height; y += 32)
        {
            data0 = displayfb + _stride * (y + phase);
            for (int x = _stride; x > 0; --x)
            {
                SPI.write(~*data0++);
            }
            data0 = data0 + stride16;
        }

        // 2. LATCH DATA
        digitalWrite(_pins.pin_latch, HIGH);
        digitalWrite(_pins.pin_latch, LOW);

        // 3. ATUR ALAMAT BARIS
        digitalWrite(_pins.pin_A, bitRead(phase, 0));
        digitalWrite(_pins.pin_B, bitRead(phase, 1));
        digitalWrite(_pins.pin_C, bitRead(phase, 2));
        digitalWrite(_pins.pin_D, bitRead(phase, 3));

        // 4. UNBLANKING DENGAN DURASI SKALA PRESISI
        if (brightenss > 0)
        {
            digitalWrite(_pins.pin_oe, LOW); // Nyalakan layar

            if (brightenss < 255)
            {
                // Skala 1-254 dipetakan ke 1-180 us
                uint16_t onTime = map(brightenss, 1, 254, 1, 180);
                delayMicroseconds(onTime);
                digitalWrite(_pins.pin_oe, HIGH); // Padamkan setelah durasi habis
            }
            // Jika brightenss == 255, OE tetap LOW terus sampai refresh berikutnya (100% Max Brightness)
        }

        phase = (phase + 1) & 0x0F;
    }
    else // PANEL_HUB12
    {
        // 1. BLANKING: Matikan OE sebelum kirim data
        digitalWrite(_pins.pin_oe, LOW);

        // Pastikan pin data konektor yang tidak dipakai selalu ditahan HIGH (Padam)
        if (_maskUnusedData)
        {
            GPOS = _maskUnusedData;
        }

        int stride4 = _stride * 4;

        if (_scanMode == SCAN_ZIGZAG)
        {
            // =========================================================================
            // MODE 1: SCAN_ZIGZAG (DEFAULT - 1 KONEKTOR VIA HARDWARE SPI)
            // =========================================================================
            volatile uint8_t *data0;
            volatile uint8_t *data1;
            volatile uint8_t *data2;
            volatile uint8_t *data3;
            bool flipRow = ((_height & 0x10) == 0);

            for (byte y = 0; y < _height; y += 16)
            {
                if (!flipRow)
                {
                    data0 = displayfb + _stride * (y + phase);
                    data1 = data0 + stride4;
                    data2 = data1 + stride4;
                    data3 = data2 + stride4;
                    for (int x = _stride; x > 0; --x)
                    {
                        SPI.write(*data3++);
                        SPI.write(*data2++);
                        SPI.write(*data1++);
                        SPI.write(*data0++);
                    }
                    flipRow = true;
                }
                else
                {
                    data0 = displayfb + _stride * (y + 16 - phase) - 1;
                    data1 = data0 - stride4;
                    data2 = data1 - stride4;
                    data3 = data2 - stride4;
                    for (int x = _stride; x > 0; --x)
                    {
                        SPI.transfer(pgm_read_byte(&(flipBits[*data3--])));
                        SPI.transfer(pgm_read_byte(&(flipBits[*data2--])));
                        SPI.transfer(pgm_read_byte(&(flipBits[*data1--])));
                        SPI.transfer(pgm_read_byte(&(flipBits[*data0--])));
                    }
                    flipRow = false;
                }
            }
        }
        else
        {
            // =========================================================================
            // MODE 2: SCAN_PARALLEL (MULTI-KONEKTOR 1, 2, ATAU 3 BARIS TANPA ZIGZAG)
            // Baris 1 (Atas)   -> pin_data1
            // Baris 2 (Tengah) -> pin_data2
            // Baris 3 (Bawah)  -> pin_data3
            // =========================================================================
            int row16Offset = _stride * 16;

            // Pointer Baris Panel 1 (Y: 0..15)
            volatile uint8_t *r0_d0 = displayfb + _stride * phase;
            volatile uint8_t *r0_d1 = r0_d0 + stride4;
            volatile uint8_t *r0_d2 = r0_d1 + stride4;
            volatile uint8_t *r0_d3 = r0_d2 + stride4;

            if (_panelsHigh == 1)
            {
                // Jika diset 1 baris pada mode SCAN_PARALLEL, baris 2 & 3 otomatis diisi HIGH (Mati)
                for (int x = _stride; x > 0; --x)
                {
                    shiftOutParallel1(*r0_d3++, _maskD0, _maskCLK, _maskUnusedData);
                    shiftOutParallel1(*r0_d2++, _maskD0, _maskCLK, _maskUnusedData);
                    shiftOutParallel1(*r0_d1++, _maskD0, _maskCLK, _maskUnusedData);
                    shiftOutParallel1(*r0_d0++, _maskD0, _maskCLK, _maskUnusedData);
                }
            }
            else if (_panelsHigh == 2)
            {
                // Pointer Baris Panel 2 (Y: 16..31)
                volatile uint8_t *r1_d0 = r0_d0 + row16Offset;
                volatile uint8_t *r1_d1 = r1_d0 + stride4;
                volatile uint8_t *r1_d2 = r1_d1 + stride4;
                volatile uint8_t *r1_d3 = r1_d2 + stride4;

                uint32_t maskAllActive = _maskD0 | _maskD1;
                for (int x = _stride; x > 0; --x)
                {
                    shiftOutParallel2(*r0_d3++, *r1_d3++, _maskD0, _maskD1, _maskCLK, maskAllActive, _maskUnusedData);
                    shiftOutParallel2(*r0_d2++, *r1_d2++, _maskD0, _maskD1, _maskCLK, maskAllActive, _maskUnusedData);
                    shiftOutParallel2(*r0_d1++, *r1_d1++, _maskD0, _maskD1, _maskCLK, maskAllActive, _maskUnusedData);
                    shiftOutParallel2(*r0_d0++, *r1_d0++, _maskD0, _maskD1, _maskCLK, maskAllActive, _maskUnusedData);
                }
            }
            else if (_panelsHigh >= 3)
            {
                // Pointer Baris Panel 2 (Y: 16..31)
                volatile uint8_t *r1_d0 = r0_d0 + row16Offset;
                volatile uint8_t *r1_d1 = r1_d0 + stride4;
                volatile uint8_t *r1_d2 = r1_d1 + stride4;
                volatile uint8_t *r1_d3 = r1_d2 + stride4;

                // Pointer Baris Panel 3 (Y: 32..47)
                volatile uint8_t *r2_d0 = r1_d0 + row16Offset;
                volatile uint8_t *r2_d1 = r2_d0 + stride4;
                volatile uint8_t *r2_d2 = r2_d1 + stride4;
                volatile uint8_t *r2_d3 = r2_d2 + stride4;

                uint32_t maskAllActive = _maskD0 | _maskD1 | _maskD2;
                for (int x = _stride; x > 0; --x)
                {
                    shiftOutParallel3(*r0_d3++, *r1_d3++, *r2_d3++, _maskD0, _maskD1, _maskD2, _maskCLK, maskAllActive);
                    shiftOutParallel3(*r0_d2++, *r1_d2++, *r2_d2++, _maskD0, _maskD1, _maskD2, _maskCLK, maskAllActive);
                    shiftOutParallel3(*r0_d1++, *r1_d1++, *r2_d1++, _maskD0, _maskD1, _maskD2, _maskCLK, maskAllActive);
                    shiftOutParallel3(*r0_d0++, *r1_d0++, *r2_d0++, _maskD0, _maskD1, _maskD2, _maskCLK, maskAllActive);
                }
            }
        }

        // 2. LATCH DATA
        digitalWrite(_pins.pin_latch, HIGH);
        digitalWrite(_pins.pin_latch, LOW);

        // 3. ATUR ALAMAT BARIS
        digitalWrite(_pins.pin_A, bitRead(phase, 0));
        digitalWrite(_pins.pin_B, bitRead(phase, 1));

        // 4. UNBLANKING DENGAN DURASI SKALA PRESISI
        if (brightenss > 0)
        {
            digitalWrite(_pins.pin_oe, HIGH); // Nyalakan layar

            if (brightenss < 255)
            {
                // Skala 1-254 dipetakan ke 1-180 us
                uint16_t onTime = map(brightenss, 1, 254, 1, 180);
                delayMicroseconds(onTime);
                digitalWrite(_pins.pin_oe, LOW); // Padamkan setelah durasi habis
            }
            // Jika brightenss == 255, OE tetap HIGH terus sampai refresh berikutnya (100% Max Brightness)
        }

        phase = (phase + 1) & 0x03;
    }
}
//----------------------------------------------------------------------------------------------
void FansElectronics_DMD8266::setBrightness(uint8_t b)
{
    if (b == 1)
    {
        b = 2;
    }
    brightenss = b;
}

//----------------------------------------------------------------------------------------------
FansElectronics_DMD8266::Color FansElectronics_DMD8266::fromRGB(uint8_t r, uint8_t g, uint8_t b)
{
    if (r || g || b)
    {
        return White;
    }
    else
    {
        return Black;
    }
}

//----------------------------------------------------------------------------------------------
void FansElectronics_DMD8266::setDoubleBuffer(bool doubleBuffer)
{
    if (doubleBuffer != _doubleBuffer)
    {
        _doubleBuffer = doubleBuffer;
        if (doubleBuffer)
        {
            // ID: Alokasi framebuffer bayangan sekunder di RAM internal
            // EN: Allocation of background rendering framebuffer in RAM
            unsigned int size = _stride * _height;
            fb1 = (uint8_t *)malloc(size);
            if (fb1)
            {
                memset(fb1, 0xFF, size);
                noInterrupts(); // ID: Matikan interupsi saat menukar memori pointer | EN: Guard critical pointer swapping
                fb = fb1;
                displayfb = fb0;
                interrupts();
            }
            else
            {
                _doubleBuffer = false;
            }
        }
        else if (fb1)
        {
            noInterrupts();
            fb = fb0;
            displayfb = fb0;
            interrupts();
            free(fb1);
            fb1 = 0;
        }
    }
}

//----------------------------------------------------------------------------------------------
void FansElectronics_DMD8266::swapBuffers()
{
    if (_doubleBuffer)
    {
        cli();
        if (fb == fb0)
        {
            fb = fb1;
            displayfb = fb0;
        }
        else
        {
            fb = fb0;
            displayfb = fb1;
        }
        sei();
    }
}

//----------------------------------------------------------------------------------------------
void FansElectronics_DMD8266::swapBuffersAndCopy()
{
    swapBuffers();
    if (_doubleBuffer)
        memcpy(fb, displayfb, _stride * _height);
}

//----------------------------------------------------------------------------------------------