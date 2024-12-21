#define BLYNK_TEMPLATE_ID "TMPL6CPI7pwpx"
#define BLYNK_TEMPLATE_NAME "MonitorAir"
#define BLYNK_AUTH_TOKEN "CDEOjT_3nDsV2bSs8brro4tdjLer_kI8"

#define BLYNK_PRINT Serial
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <NewPing.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Data autentikasi Blynk dan WiFi
char auth[] = "CDEOjT_3nDsV2bSs8brro4tdjLer_kI8";
char ssid[] = "Lab Robotika";
char pass[] = "lab_robotika";

// Konfigurasi pin
#define TRIG_PIN 12
#define ECHO_PIN 14
#define RELAY_PIN_PUMP 19 // Pin untuk relay pompa

// Inisialisasi LCD dan sensor ultrasonik
LiquidCrystal_PCF8574 lcd(0x27);
NewPing sonar(TRIG_PIN, ECHO_PIN, 200); 

// Konstanta untuk tinggi sensor dari tanah
const int sensorHeight = 50;  // Tinggi sensor dari tanah (cm)

// Variabel untuk menyimpan jarak dan kontrol
int distance = 0;
int waterHeight = 0; // Ketinggian air dari tanah
bool pumpStatus = false; // Status pompa, true = ON, false = OFF
bool autoMode = true; // Status mode otomatis, true = otomatis aktif

BlynkTimer timer;

// Fungsi untuk membaca dan mengirim data ketinggian air ke Blynk
void checkWaterLevel() {
    if (autoMode) {
        distance = sonar.ping_cm(); 
        waterHeight = sensorHeight - distance; // Hitung ketinggian air dari tanah

        Blynk.virtualWrite(V1, waterHeight); 

        // Tampilkan di LCD hanya jika mode otomatis aktif
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Air: ");
        lcd.print(waterHeight);
        lcd.print(" cm");

        // Logika kontrol pompa otomatis
        if (waterHeight >= 40 && !pumpStatus) {
            digitalWrite(RELAY_PIN_PUMP, LOW); // Pompa menyala
            pumpStatus = true;
            Serial.println("Pompa ON (Air > 40 cm)");
        } 
        else if (waterHeight <= 20 && pumpStatus) {
            digitalWrite(RELAY_PIN_PUMP, HIGH); // Pompa mati
            pumpStatus = false;
            Serial.println("Pompa OFF (Air < 20 cm)");
        }

        lcd.setCursor(0, 1);
        lcd.print("Pompa: ");
        lcd.print(pumpStatus ? "ON " : "OFF");
    }
}

// Fungsi untuk mengaktifkan atau menonaktifkan mode otomatis melalui Blynk (V16)
BLYNK_WRITE(V16) {
    autoMode = !param.asInt(); // OFF (0) di V16 artinya mode otomatis aktif; ON (1) artinya manual

    if (autoMode) {
        Serial.println("Mode Otomatis: ON");
        pumpStatus = false; // Reset pompa saat mode otomatis aktif
        Blynk.virtualWrite(V15, 0); // Reset kontrol manual di aplikasi Blynk
    } else {
        Serial.println("Mode Otomatis: OFF - Manual Mode Aktif");
        digitalWrite(RELAY_PIN_PUMP, HIGH); // Pastikan pompa mati saat berpindah ke Manual Mode
        // Menampilkan mode manual di LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Mode Manual Aktif");
        lcd.setCursor(0, 1);
        lcd.print("Pompa: ");
        lcd.print(pumpStatus ? "ON " : "OFF");
    }
}

// Fungsi untuk kontrol manual pompa melalui Blynk (V15)
BLYNK_WRITE(V15) {
    if (!autoMode) { // Hanya izinkan kontrol manual jika mode otomatis OFF
        bool manualControl = param.asInt(); // 1 untuk ON, 0 untuk OFF

        if (manualControl) {
            digitalWrite(RELAY_PIN_PUMP, LOW); // Nyalakan pompa
            pumpStatus = true;
            Serial.println("Kontrol Manual: Pompa ON");
        } else {
            digitalWrite(RELAY_PIN_PUMP, HIGH); // Matikan pompa
            pumpStatus = false;
            Serial.println("Kontrol Manual: Pompa OFF");
        }

        // Perbarui status pompa manual di LCD
        lcd.setCursor(0, 1);
        lcd.print("Pompa: ");
        lcd.print(pumpStatus ? "ON " : "OFF");
    } else {
        Serial.println("Mode Otomatis Aktif - Kontrol Manual Tidak Tersedia");
        Blynk.virtualWrite(V15, 0); // Reset tombol manual di Blynk ke OFF
    }
}

void setup() {
    Serial.begin(9600);
    lcd.begin(16, 2, Wire);
    lcd.setBacklight(255);
    pinMode(RELAY_PIN_PUMP, OUTPUT);

    Blynk.begin(auth, ssid, pass);
    timer.setInterval(1000L, checkWaterLevel); // Interval 1 detik untuk update ketinggian air

    Serial.println("Monitoring...");
    digitalWrite(RELAY_PIN_PUMP, HIGH); // Pompa mati di awal
}

void loop() {
    Blynk.run();
    timer.run();
}
