#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <TFT_eSPI.h>
#include <vector>
#include <WiFi.h>
#include <Wire.h>
#include <HTTPClient.h> 
#include <WiFiClientSecure.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>


#define RST_PIN   13
#define SS_PIN    33
#define SCK_PIN   25
#define MISO_PIN  27
#define MOSI_PIN  26

#define BUTTON_PIN 0
#define GREEN_PIN 2
#define RED_PIN 17

/*
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
*/

MFRC522 mfrc522(SS_PIN, RST_PIN);
TFT_eSPI tft = TFT_eSPI();

char ssid[] = "iPhone";
char pass[] = "ajHsb123";

const char* server = "https://uniqlo2.vercel.app/api/checkout/total"; // <-- Full URL for HTTPClient

std::vector<uint64_t> scannedUIDs; //updated to store unsigned 64 bit int - instead of strings for hex

void blinkLED() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(RED_PIN, LOW);
    delay(100);
    digitalWrite(RED_PIN, HIGH);
    delay(100);
    digitalWrite(RED_PIN, LOW);
  }
  digitalWrite(GREEN_PIN, HIGH);
  //delay(10000);
  //digitalWrite(GREEN_PIN, LOW);
  //digitalWrite(RED_PIN, HIGH);
}

void sendUIDToServer(uint64_t uid) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClientSecure client;

    client.setInsecure(); // Disable SSL certificate verification

    http.begin(client, server); // full URL with secure client
    http.addHeader("Content-Type", "application/json");
    
    // building JSON array
    String postData = "{\"tagIds\": [";
    for (size_t i = 0; i < scannedUIDs.size(); i++) {
      //postData += "\"" + scannedUIDs[i] + "\"";
      postData += String(scannedUIDs[i]);
      if (i < scannedUIDs.size() - 1) {
        postData += ",";
      }
    }
    postData += "]}";
    Serial.println(postData);
    //////////////////////
    //String postData = "{\"tagIds\": [" + String(uid) + "]}"; // sending string at a time 
    
    Serial.println("Sending UID to server...");
    Serial.println(postData);

    int httpResponseCode = http.POST(postData);

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    String response = http.getString();
    Serial.println("Server response: " + response);

    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(RED_PIN, HIGH);
    //scannedUIDs.clear(); // Clear the scanned UIDs after sending to server

    http.end();
  } else {
    Serial.println("WiFi not connected!");
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  //digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(RED_PIN, HIGH);
  
  /*
  Wire.begin(21, 22); // SDA, SCL
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for(;;); // halt
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("RFID Ready");
  display.display();
  */

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
  // create conditional for button press 
  if (digitalRead(BUTTON_PIN) == LOW) {
    blinkLED();
    sendUIDToServer(0); // Send UID 0 when button is pressed
    scannedUIDs.clear();
    //delay(1000); // Debounce delay
  }


  for (size_t i = 0; i < scannedUIDs.size(); i++) {
    tft.setCursor(0, 15 + (i * 12));
    tft.println(scannedUIDs[i]);
  }

  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(500);
    return;
  }
  /*
  String uidStr = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uidStr.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    uidStr.concat(String(mfrc522.uid.uidByte[i], HEX));
    if (i < mfrc522.uid.size - 1) uidStr.concat(" ");
  }
  uidStr.toUpperCase();
  */
  uint64_t uidValue = 0;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i]);
    Serial.print(i < mfrc522.uid.size - 1 ? " " : "\n");
    uidValue = (uidValue << 8) | mfrc522.uid.uidByte[i]; 
  }

  bool alreadyScanned = false;
  for (const auto& uid : scannedUIDs) {
    if (uid == uidValue) {
      alreadyScanned = true;
      Serial.println("Duplicate UID, skipping...");
      break;
    }
  }

  if (!alreadyScanned) {
    Serial.print("New UID: ");
    Serial.println(uidValue);
    scannedUIDs.push_back(uidValue);
    /*
    uint64_t fullUID = 0; // construct the full hex representation; use 64 bits to support full hex length 
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      fullUID = fullUID << 8 | mfrc522.uid.uidByte[i]; // make space for the next byte, then write current byte to first 8 bits
    }
    */

    //sendUIDToServer(uidValue); // Send the UID to the server
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(1000);
}
