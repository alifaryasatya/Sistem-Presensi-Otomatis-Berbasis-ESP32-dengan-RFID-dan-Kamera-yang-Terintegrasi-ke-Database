#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SS_PIN 5
#define RST_PIN 4
#define BUZZER_PIN 25
#define LCD_POWER_PIN 27

MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

struct Pengguna {
  String uid;
  String nama;
};

Pengguna daftarPengguna[] = {
  {"231BA8E", "ANDI FADHLI"},
  {"3F98A252", "RAFAEL SADEWO"},
  {"BF7CA252", "ABIDI UTAMA"}
};

const char* ssid = "Walisongo";
const char* password = "Songosongo";

unsigned long lastActivityTime = 0;
const unsigned long idleTimeout = 10000; // 10 detik
bool isStandby = false;

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LCD_POWER_PIN, OUTPUT);
  digitalWrite(LCD_POWER_PIN, HIGH); // Nyalakan LCD

  SPI.begin();
  mfrc522.PCD_Init();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Scan Kartu RFID");

  connectToWiFi();
  lastActivityTime = millis();
}

void loop() {
  if (isStandby && mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    wakeUpFromStandby();
  }

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();
    Serial.print("UID Kartu: ");
    Serial.println(uid);

    String nama = getNamaByUID(uid);
    if (nama != "") {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Akses Diterima");
      lcd.setCursor(0, 1);
      lcd.print(nama);
      tone(BUZZER_PIN, 500, 1000);
      uploadAbsensi(uid, nama);
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Akses Ditolak!");
      for (int i = 0; i < 2; i++) {
        tone(BUZZER_PIN, 500, 300);
        delay(400);
      }
    }

    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scan Kartu RFID");
    lastActivityTime = millis();
  }

  if (!isStandby && millis() - lastActivityTime > idleTimeout) {
    enterStandbyMode();
  }
}

String getNamaByUID(String uid) {
  for (int i = 0; i < sizeof(daftarPengguna) / sizeof(daftarPengguna[0]); i++) {
    if (daftarPengguna[i].uid == uid) {
      return daftarPengguna[i].nama;
    }
  }
  return "";
}

void uploadAbsensi(String uid, String name) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverURL = "https://script.google.com/macros/s/AKfycby9_t8UoFun8c-7Rxgmt3M0RbsIoPI5Z7kyOHd2RJozeV1V33-xtloBP-ZAdcpK9TKCmQ/exec";
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String postData = "uid=" + uid + "&name=" + name;
    int httpCode = http.POST(postData);
    if (httpCode > 0) {
      Serial.printf("HTTP Response code: %d\n", httpCode);
      String payload = http.getString();
      Serial.println(payload);
    } else {
      Serial.println("Error in HTTP request");
    }
    http.end();
  } else {
    Serial.println("Tidak terhubung WiFi");
  }
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Tersambung!");
}

void enterStandbyMode() {
  Serial.println("Masuk standby mode");

  digitalWrite(LCD_POWER_PIN, LOW); // Matikan LCD
  WiFi.disconnect(true);
  isStandby = true;

  // Masuk light sleep selama 10 detik
  esp_sleep_enable_timer_wakeup(10 * 1000000);
  esp_light_sleep_start();
}

void wakeUpFromStandby() {
  Serial.println("Bangun dari standby");

  digitalWrite(LCD_POWER_PIN, HIGH); // Nyalakan LCD
  delay(100); // Kasih waktu LCD nyala
  lcd.init();
  lcd.backlight();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan Kartu RFID");

  connectToWiFi();
  isStandby = false;
  lastActivityTime = millis();
}
