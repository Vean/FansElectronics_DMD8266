/*
 * Creator             : Irfan Indra Kurniawan, ST
 * Website             : https://www.fanselectronics.com
 * Buy Me a Coffee     : https://saweria.co/FansElectronics
 * Tested on Wemos D1 mini ESP8266-12E with HUB12 Multi-Row Panels
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

// ID: KONFIGURASI PIN UNTUK TIPE HUB12 MULTI-KONEKTOR (PARALEL)
// EN: PIN CONFIGURATION FOR HUB12 MULTI-CONNECTOR (PARALLEL MODE)
DmdPins pinsHUB12 = {
    .pin_A = 16,     // ID: D0 (GPIO16) -> Pin Alamat Baris A | EN: Scan Line Address A
    .pin_B = 12,     // ID: D6 (GPIO12) -> Pin Alamat Baris B | EN: Scan Line Address B
    .pin_C = -1,     // ID: Tidak dipakai di HUB12 | EN: Not used on HUB12
    .pin_D = -1,     // ID: Tidak dipakai di HUB12 | EN: Not used on HUB12
    .pin_latch = 0,  // ID: D3 (GPIO0)  -> STB / LATCH | EN: Latch strobe line
    .pin_sck = 14,   // ID: D5 (GPIO14) -> CLK / SCK | EN: Shift register clock line
    .pin_oe = 15,    // ID: D8 (GPIO15) -> nOE (Enable Output) | EN: Output Enable line
    .pin_data1 = 13, // ID: D7 (GPIO13) -> DATA Konektor Baris 1 | EN: DATA Connector Row 1
    .pin_data2 = 2,  // ID: D4 (GPIO2)  -> DATA Konektor Baris 2 | EN: DATA Connector Row 2
    .pin_data3 = 3   // ID: RX (GPIO3)  -> DATA Konektor Baris 3 | EN: DATA Connector Row 3
};

// ID: Atur jumlah baris fisik panel (2 atau 3 baris)
// EN: Set physical row count (2 or 3 rows)
const int ROW_COUNT = 3;

void setup()
{
    // -------------------------------------------------------------------------
    // ID: Inisialisasi: 1 Panel Lebar (32px) x ROW_COUNT Baris Tinggi, Mode SCAN_PARALLEL
    // EN: Initialization: 1 Panel Wide (32px) x ROW_COUNT Rows High, SCAN_PARALLEL Mode
    // -------------------------------------------------------------------------
    dmd.begin(PANEL_HUB12, 1, ROW_COUNT, pinsHUB12, SCAN_PARALLEL);

    // ID: Mengaktifkan Double Buffer untuk render di latar belakang (Anti-Kedip)
    // EN: Enable Double Buffer for background rendering (Flicker-Free)
    dmd.setDoubleBuffer(true);

    // ID: Mengatur tingkat kecerahan LED (0 = Mati, 255 = Maksimal)
    // EN: Set LED brightness level (0 = Off, 255 = Maximum)
    dmd.setBrightness(140);
}

void loop()
{
    // ID: Bersihkan buffer memori latar belakang sebelum digambar ulang
    // EN: Clear background memory buffer before redrawing
    dmd.clear();

    // ID: Jalankan teks berjalan di Baris 1 (Y=4, Kecepatan 35ms)
    // EN: Execute running text on Row 1 (Y=4, Speed 35ms)
    dmd.setFont(SystemFont5x7);
    runningText(0, 4, 35, "BARIS 1: RUNNING TEXT HUB12 PARALLEL ");

    // ID: Jalankan teks berjalan di Baris 2 (Y=20, Kecepatan 25ms) jika minimal 2 baris aktif
    // EN: Execute running text on Row 2 (Y=20, Speed 25ms) if at least 2 rows active
    if (ROW_COUNT >= 2)
    {
        dmd.setFont(System6x7);
        runningText(1, 20, 25, "BARIS 2: ESP8266 FLICKER-FREE DEMO ");
    }

    // ID: Jalankan teks berjalan di Baris 3 (Y=36, Kecepatan 45ms) jika 3 baris aktif
    // EN: Execute running text on Row 3 (Y=36, Speed 45ms) if 3 rows active
    if (ROW_COUNT >= 3)
    {
        dmd.setFont(SystemFont5x7);
        runningText(2, 36, 45, "BARIS 3: FANSELECTRONICS DMD8266 OK ");
    }

    // ID: Tukar buffer memori bayangan ke memori tampilan aktif secara instan
    // EN: Swap background memory buffer to active display memory instantly
    dmd.swapBuffers();

    // ID: Trigger fungsi interupsi untuk memindai fisik baris LED
    // EN: Trigger interrupt function to scan physical LED rows
    dmd.update();
}

// ID: FUNGSI PEMBANTU: ANIMASI TEKS BERJALAN MULTI-BARIS INDEPENDEN
// EN: HELPER FUNCTION: INDEPENDENT MULTI-ROW RUNNING TEXT ANIMATION
void runningText(int rowIdx, int y, uint8_t speed, const String &text)
{
    static uint32_t lastScroll[3] = {0, 0, 0};
    static int posX[3] = {0, 0, 0};

    uint32_t currentMillis = millis();
    int totalWidth = dmd.textWidth(text);

    // ID: Periksa pewaktu pergeseran piksel teks
    // EN: Evaluate text pixel shifting timer
    if ((currentMillis - lastScroll[rowIdx]) > speed)
    {
        lastScroll[rowIdx] = currentMillis;
        posX[rowIdx]++;
        if (posX[rowIdx] > (dmd.width() + totalWidth))
        {
            // ID: Reset ke awal jika teks telah tuntas | EN: Reset to start when text finished
            posX[rowIdx] = 0;
        }
    }

    // ID: Render teks bergerak dari sisi kanan ke kiri kanvas
    // EN: Render moving text from right to left across canvas
    dmd.drawText(dmd.width() - posX[rowIdx], y, text);
}