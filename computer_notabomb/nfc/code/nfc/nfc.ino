#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN  10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);

// ====================================
//   UID de la bonne carte
// ====================================
byte uidAutorise[] = {0x1F, 0x2D, 0xDF, 0x02};
byte tailleUid = 4;
// ====================================

void setup() {
  Serial.begin(9600);
  delay(1000);
  SPI.begin();
  rfid.PCD_Init();
  rfid.PCD_SetAntennaGain(rfid.RxGain_max);
  Serial.println("Approchez votre carte...");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  Serial.print("UID detecte : ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) Serial.print("0");
    Serial.print(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) Serial.print(":");
  }
  Serial.println();

  bool autorise = (rfid.uid.size == tailleUid);
  for (byte i = 0; i < tailleUid && autorise; i++) {
    if (rfid.uid.uidByte[i] != uidAutorise[i]) autorise = false;
  }

  if (autorise) {
    Serial.println(">>> ACCES AUTORISE <<<");
  } else {
    Serial.println(">>> ACCES REFUSE <<<");
  }

  rfid.PICC_HaltA();
  delay(2000);
}
