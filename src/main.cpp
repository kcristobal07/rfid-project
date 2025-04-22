#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <TFT_eSPI.h>
#include <vector>
#include <WiFi.h>
#include <Wire.h>
#include <HTTPClient.h> // <-- This one, not HttpClient.h!
#include <WiFiClientSecure.h>

#define RST_PIN   13
#define SS_PIN    33
#define SCK_PIN   25
#define MISO_PIN  27
#define MOSI_PIN  26

MFRC522 mfrc522(SS_PIN, RST_PIN);
TFT_eSPI tft = TFT_eSPI();

char ssid[] = "iPhone";
char pass[] = "ajHsb123";

const char* server = "https://uniqlo2.vercel.app/api/checkout/total"; // <-- Full URL for HTTPClient

std::vector<String> scannedUIDs;

void sendUIDToServer(uint64_t uid) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClientSecure client;

    client.setInsecure(); // Disable SSL certificate verification

    http.begin(client, server); // full URL with secure client
    http.addHeader("Content-Type", "application/json");

    String postData = "{\"tagIds\": [" + String(uid) + "]}";

    Serial.println(postData);

    int httpResponseCode = http.POST(postData);

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    String response = http.getString();
    Serial.println("Server response: " + response);

    http.end();
  } else {
    Serial.println("WiFi not connected!");
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  WiFi.begin(ssid, pass);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: ");
  Serial.println(WiFi.localIP());

  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  mfrc522.PCD_Init();
  delay(100);
  Serial.println("RFID Init complete. Tap your card...");
}

void loop() {
  for (size_t i = 0; i < scannedUIDs.size(); i++) {
    tft.setCursor(0, 15 + (i * 12));
    tft.println(scannedUIDs[i]);
  }

  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(500);
    return;
  }

  String uidStr = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uidStr.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    uidStr.concat(String(mfrc522.uid.uidByte[i], HEX));
    if (i < mfrc522.uid.size - 1) uidStr.concat(" ");
  }
  uidStr.toUpperCase();

  bool alreadyScanned = false;
  for (const auto& uid : scannedUIDs) {
    if (uid == uidStr) {
      alreadyScanned = true;
      Serial.println("Duplicate UID, skipping...");
      break;
    }
  }

  if (!alreadyScanned) {
    Serial.println("New UID: " + uidStr);
    scannedUIDs.push_back(uidStr);

    uint64_t fullUID = 0; // construct the full hex representation; use 64 bits to support full hex length
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      fullUID = fullUID << 8 | mfrc522.uid.uidByte[i]; // make space for the next byte, then write current byte to first 8 bits
    }

    sendUIDToServer(fullUID);
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(1000);
}
