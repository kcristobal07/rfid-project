#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <TFT_eSPI.h>
#include <vector>
#include <WiFi.h>
#include <Wire.h>
#include <HTTPClient.h> 
#include <WiFiClientSecure.h>
#include <User_Setups/Setup25_TTGO_T_Display.h>

#define RST_PIN   13
#define SS_PIN    33
#define SCK_PIN   25
#define MISO_PIN  27
#define MOSI_PIN  26

#define BUTTON_PIN 0
#define BUTTON_PURCHASE 35

#define GREEN_PIN 2
#define RED_PIN 17

MFRC522 mfrc522(SS_PIN, RST_PIN);
TFT_eSPI tft = TFT_eSPI();

char ssid[] = "iPhone";
char pass[] = "ajHsb123";

//const char* server = "https://uniqlo2.vercel.app/api/checkout/total"; // <-- Full URL for HTTPClient

// new constants 
const char* subtotalServer = "https://uniqlo2.vercel.app/api/checkout/total"; // <-- Full URL for HTTPClient
const char* purchaseServer = "https://uniqlo2.vercel.app/api/checkout/purchase"; // <-- Full URL for HTTPClient

std::vector<uint64_t> scannedUIDs; //updated to store unsigned 64 bit int - instead of strings for hex
bool purchaseMode = false; // Flag to indicate if purchase mode is active - START with subtotal mode


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

void requestSubtotal() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClientSecure client;

    client.setInsecure(); // Disable SSL certificate verification

    http.begin(client, subtotalServer); // full URL with secure client
    http.addHeader("Content-Type", "application/json");
    
    String postData = "{\"tagIds\": [";
    for (size_t i = 0; i < scannedUIDs.size(); i++) {
      postData += String(scannedUIDs[i]);
      if (i < scannedUIDs.size() - 1) {
        postData += ",";
      }
    }
    postData += "]}";

    int httpResponseCode = http.POST(postData);
    String response = http.getString();
Serial.println("Subtotal response: " + response);

// Extract the "total" value manually
int totalIndex = response.indexOf("\"total\":");
if (totalIndex != -1) {
  int numberStart = totalIndex + 8; // after "total":
  int numberEnd = response.indexOf("}", numberStart);
  String totalStr = response.substring(numberStart, numberEnd);

  Serial.println("Parsed total value: " + totalStr);

  bool isNumber = true;
  for (int i = 0; i < totalStr.length(); i++) {
    if (!isDigit(totalStr[i]) && totalStr[i] != '.' && totalStr[i] != '-') {
      isNumber = false;
      break;
    }
  }

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(4);

  if (isNumber && totalStr.length() > 0) {
    double value = totalStr.toDouble();  // Now parse extracted number
    tft.println("Subtotal:");
    tft.print("$");
    tft.print(String(value, 2));          // 2 decimal places
  } else {
    tft.println("Server says:");
    tft.println(response);
  }
} else {
  // Could not find "total" field
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextSize(2);
  tft.println("Subtotal not found!");
}

    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(RED_PIN, HIGH);

    http.end();
  } else {
    Serial.println("WiFi not connected!");
  }
}

void sendPurchase() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClientSecure client;

    client.setInsecure(); // Disable SSL certificate verification

    http.begin(client, purchaseServer); // full URL with secure client
    http.addHeader("Content-Type", "application/json");
    
    // building JSON array
    String postData = "{\"tagIds\": [";
    for (size_t i = 0; i < scannedUIDs.size(); i++) {
      postData += String(scannedUIDs[i]);
      if (i < scannedUIDs.size() - 1) {
        postData += ",";
      }
    }
    postData += "]}";
    Serial.println(postData);
    
    Serial.println("Sending Purchase to server...");
    Serial.println(postData);

    int httpResponseCode = http.POST(postData);

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    String response = http.getString();
    Serial.println("Purchase response: " + response);

    int idStart = response.indexOf("\"id\":") + 5;
    int idEnd = response.indexOf(",", idStart);
    String orderIdStr = response.substring(idStart, idEnd);

    int dateStart = response.indexOf("\"created_at\":\"") + 14;
    int dateEnd = response.indexOf("\"", dateStart);
    String createdAtStr = response.substring(dateStart, dateEnd);

    Serial.println("Parsed Order ID: " + orderIdStr);
    Serial.println("Parsed Created At: " + createdAtStr);

    tft.fillScreen(TFT_BLACK);             // Clear screen
    tft.setCursor(0, 0);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(4);
    tft.println("Purchased!");

    tft.setTextSize(2);
    tft.println("Order ID:");
    tft.println(orderIdStr);

    tft.println("Date:");
    tft.println(createdAtStr.substring(0, 10)); // show as YYYY-MM-DD

    delay(10000); // Wait a few seconds to show outcome (reciept)
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextSize(4);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println("RFID Ready");

    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(RED_PIN, HIGH);

    http.end();
  } else {
    Serial.println("WiFi not connected!");
  }
}



/*
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

    tft.fillScreen(TFT_BLACK);             // Clear screen
    tft.setCursor(0, 0);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(4);

    // Try to extract a numeric value
    bool isNumber = true;
    for (int i = 0; i < response.length(); i++) {
      if (!isDigit(response[i]) && response[i] != '.' && response[i] != '-') {
        isNumber = false;
        break;
      }
    }

    if (isNumber && response.length() > 0) {
      double value = response.toDouble();  // Convert string to double
      tft.println("Subtotal:");
      tft.print("$");
      tft.print(String(value, 2));          // 2 decimal places
    } else {
      tft.println("Server says:");
      int cursorY = 30;
      int chunkSize = 20;

      for (int i = 0; i < response.length(); i += chunkSize) {
        tft.setCursor(0, cursorY);
        tft.println(response.substring(i, i + chunkSize));
        cursorY += 20;
      }
    }
    // Now print the response text
    int cursorY = 30;
    int chunkSize = 20; // split long strings if needed

    for (int i = 0; i < response.length(); i += chunkSize) {
      tft.setCursor(0, cursorY);
      tft.println(response.substring(i, i + chunkSize));
      cursorY += 20; // move down for next line
    }

    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(RED_PIN, HIGH);
    //scannedUIDs.clear(); // Clear the scanned UIDs after sending to server

    http.end();
  } else {
    Serial.println("WiFi not connected!");
  }
}
*/


void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PURCHASE, INPUT_PULLUP);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  //digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(RED_PIN, HIGH);
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(4);
  tft.setCursor(0, 0);
  tft.println("RFID Ready");

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
  /*
  if (digitalRead(BUTTON_PIN) == LOW) {
    blinkLED();
    if (!purchaseMode) {
      requestSubtotal();   // First click: get subtotal
      purchaseMode = true; // Switch to purchase mode
    } else {
      sendPurchase();      // Second click: complete purchase
      purchaseMode = false;
      scannedUIDs.clear(); // Clear UIDs after purchase
    }

    delay(1000); // Button debounce
  }
  */
  if (digitalRead(BUTTON_PIN) == LOW) {
    blinkLED();
    requestSubtotal();
    delay(1000);
  }
  if (digitalRead(BUTTON_PURCHASE) == LOW) {
    blinkLED();
    // purchaseMode = true; // Set purchase mode to true
    sendPurchase();
    scannedUIDs.clear(); // Clear the scanned UIDs after sending to server
    delay(1000);
  }
  /*
  for (size_t i = 0; i < scannedUIDs.size(); i++) {
    tft.setCursor(0, 15 + (i * 12));
    tft.println(scannedUIDs[i]);
  }
  */

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
