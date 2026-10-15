//----------------------------------------------------------------------------------------------
/*
 * Creator             : Irfan Indra Kurniawan, ST
 * Website             : https://www.fanselectronics.com
 * Buy Me a Coffee     : https://saweria.co/FansElectronics
 * Ported for Multi-Panel Type (HUB08 & HUB12) on ESP8266
 * Support P10, P4.25, P7.62, dan panel custom dengan konfigurasi pin fleksibel
 */

#ifndef FansElectronics_DMD8266_h
#define FansElectronics_DMD8266_h

#include "SPI.h"
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

// ID: Enumerasi tipe panel fisik matrix yang didukung
// EN: Enumeration of supported physical matrix panel types
enum PanelType
{
    PANEL_HUB08,
    PANEL_HUB12
};

// ID: Enumerasi metode pemindaian jalur data antar baris panel (Khusus HUB12)
// EN: Enumeration of data routing scan modes across panel rows (HUB12 Only)
enum ScanMode
{
    SCAN_ZIGZAG,  // Default: 1 Konektor, sambung seri/zigzag via Hardware SPI
    SCAN_PARALLEL // Multi-Konektor: Tiap baris panel beda pin DATA (Max 3 Baris di ESP8266)
};

// ID: Struktur data konfigurasi pin fleksibel untuk pemetaan GPIO
// EN: Flexible pin configuration data structure for GPIO mapping
struct DmdPins
{
    int pin_A;     // Pin untuk alamat baris Multiplexing (Scan Line Addressing)
    int pin_B;     // Pin untuk alamat baris Multiplexing (Scan Line Addressing)
    int pin_C;     // ID: Diisi -1 jika HUB12 | EN: Set to -1 if HUB12
    int pin_D;     // ID: Diisi -1 jika HUB12 | EN: Set to -1 if HUB12
    int pin_latch; // LAT / SCLK
    int pin_sck;   // CLK
    int pin_oe;    // OE / EN
    union
    {
        int pin_data1; // DATA Konektor Baris 1 (Utama / R1)
        int pin_mosi;  // Alias untuk kompatibilitas kode lama
    };
    int pin_data2; // DATA Konektor Baris 2 (Default -1 jika tidak dipakai)
    int pin_data3; // DATA Konektor Baris 3 (Default -1 jika tidak dipakai)
};

class FansElectronics_DMD8266 : public FansElectronics_Bitmap
{
public:
    // ID: Konstruktor dasar untuk alokasi memori dinamis
    // EN: Default constructor for dynamic memory allocation
    FansElectronics_DMD8266();
    ~FansElectronics_DMD8266();

    bool doubleBuffer() const { return _doubleBuffer; }
    void setDoubleBuffer(bool doubleBuffer);
    void swapBuffers();
    void swapBuffersAndCopy();

    // ID: Inisialisasi utama driver layar dengan parameter dimensi & mode scan fleksibel
    // EN: Core display driver initialization function with flexible dimension & scan mode parameters
    void begin(PanelType type, int panelsWide, int panelsHigh, DmdPins pins, int panelWidthPx = 0, int panelHeightPx = 0, ScanMode scanMode = SCAN_ZIGZAG);

    // ID: Overload fungsi begin() agar mudah memanggil ScanMode tanpa harus mengisi resolusi piksel
    // EN: Overloaded begin() helper to easily pass ScanMode without specifying custom pixel dimensions
    void begin(PanelType type, int panelsWide, int panelsHigh, DmdPins pins, ScanMode scanMode)
    {
        begin(type, panelsWide, panelsHigh, pins, 0, 0, scanMode);
    }

    void update();
    void refresh();
    void setBrightness(uint8_t b);
    static Color fromRGB(uint8_t r, uint8_t g, uint8_t b);

private:
    FansElectronics_DMD8266(const FansElectronics_DMD8266 &other) : FansElectronics_Bitmap(other) {}
    FansElectronics_DMD8266 &operator=(const FansElectronics_DMD8266 &) { return *this; }
    uint8_t brightenss;
    bool _doubleBuffer;
    uint8_t phase;
    uint8_t *fb0;
    uint8_t *fb1;
    uint8_t *displayfb;
    unsigned long lastRefresh;

    // ID: Variabel pelacak konfigurasi internal driver
    // EN: Internal driver configuration tracking variables
    PanelType _panelType;
    ScanMode _scanMode;
    DmdPins _pins;
    int _panelsWide;
    int _panelsHigh;
    int _panelWidthPx;
    int _panelHeightPx;

    // Bitmask internal untuk fast GPIO switching dan blanking otomatis
    uint32_t _maskD0;
    uint32_t _maskD1;
    uint32_t _maskD2;
    uint32_t _maskCLK;
    uint32_t _maskUnusedData;

    // ID: Fungsi internal pengirim 1 byte data paralel (1, 2, atau 3 baris sekaligus)
    // EN: Internal fast bit-banging helper for parallel multi-connector row shifting
    inline void shiftOutParallel1(uint8_t b0, uint32_t maskD0, uint32_t maskCLK, uint32_t maskUnused);
    inline void shiftOutParallel2(uint8_t b0, uint8_t b1, uint32_t maskD0, uint32_t maskD1, uint32_t maskCLK, uint32_t maskAllActive, uint32_t maskUnused);
    inline void shiftOutParallel3(uint8_t b0, uint8_t b1, uint8_t b2, uint32_t maskD0, uint32_t maskD1, uint32_t maskD2, uint32_t maskCLK, uint32_t maskAllActive);
};

#endif
//----------------------------------------------------------------------------------------------