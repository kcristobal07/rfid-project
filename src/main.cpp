#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <TFT_eSPI.h>

// Custom SPI pinsff
#define RST_PIN         13   // Reset pin
#define SS_PIN          33   // SDA/SS pin
#define SCK_PIN         25
#define MISO_PIN        27
#define MOSI_PIN        26

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

TFT_eSPI tft = TFT_eSPI(); // TFT instance

void setup() {
  Serial.begin(115200);
  delay(2000);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.println("RFID Reader");

  Serial.println("RFID Reader Starting...");

  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  mfrc522.PCD_Init();
  delay(100); // add delay to allow the reader to initialize

  Serial.println("RFID Init complete. Tap your card...");

}

void loop() {
  Serial.println("Waiting for card...");
  delay(1000);

  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print("Tag UID: ");
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();

  content.toUpperCase();
  if (content.substring(1) == "A3 3B AE 1A") { // Replace with your UID
    Serial.println("Authorized access");
    Serial.println("Clothing Piece: Wide Pleated Sweatpants");
  } else {
    Serial.println("Access denied");
  }

  // Display UID on TFT
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.println("RFID UID:");
  tft.setTextSize(3);
  tft.setCursor(0, 30);
  tft.println(content);

  // stop reading uid so new one can be detected
  // mfrc522.PICC_HaltA(); // build-in function to halt PICC
  // mfrc522.PCD_StopCrypto1();

  //remove delay to allow contrinous stream mode 
  delay(2000); // add delay to allow next tag detection
}
