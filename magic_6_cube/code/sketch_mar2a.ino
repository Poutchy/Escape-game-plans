#include <Wire.h>
#include "rgb_lcd.h"
#include "LSM6DS3.h"

rgb_lcd lcd;
LSM6DS3 myIMU(I2C_MODE, 0x6A);

const int LED_PIN = 4;
const int BUTTON_PIN = 5;
const int RESET_LED_PIN = 2;
const int RESET_BUTTON_PIN = 3;

const char* unlockedMessage = "";
int scrollPos = 0;
unsigned long lastScrollTime = 0;
const int SCROLL_INTERVAL = 300;

struct Secret {
  int code[4];
  const char* message;
};

Secret secrets[] = {
  { { 1, 1, 1, 1 }, "    Bouton rouge pour remettre un code" },
  { { 1, 3, 5, 2 }, "    La troisième cle commence par V" },
  { { 5, 6, 1, 3 }, "    Les cles encore utilisables contiennent toutes la lettre B" },
  { { 4, 6, 2, 3 }, "    La cle 1 commence par une lettre plus proche de A que celle de la 2e" },
  { { 2, 3, 4, 1 }, "    Une cle valide ne commence jamais par R" },
  { { 5, 4, 1, 6 }, "    Une seule cle fonctionnelle contient la lettre O" },
  { { 3, 2, 3, 5 }, "    Les cles valides ne commencent jamais par B" },
};
const int NUM_SECRETS = sizeof(secrets) / sizeof(secrets[0]);

const int CODE_LENGTH = 4;
int enteredCode[4];
int indexCode = 0;
bool unlocked = false;
int lastButtonState = HIGH;

const int couleurs[6][3] = {
  { 0, 255, 0 },    // Plat
  { 255, 0, 0 },    // Retourne
  { 0, 0, 255 },    // Gauche
  { 255, 255, 0 },  // Droite
  { 255, 128, 0 },  // Avant
  { 128, 0, 255 }   // Arriere
};

const char* labels[6] = { "Plat", "Retourne", "Gauche", "Droite", "Avant", "Arriere" };
const char letters[6] = { 'V', 'R', 'B', 'J', 'O', 'M' };

int getOrientation(float ax, float ay, float az) {
  float absX = abs(ax);
  float absY = abs(ay);
  float absZ = abs(az);

  int orientation;

  if (absZ > absX && absZ > absY)
    orientation = az > 0 ? 1 : 2;

  else if (absX > absY && absX > absZ)
    orientation = ax > 0 ? 5 : 6;

  else
    orientation = ay > 0 ? 4 : 3;

  switch (orientation) {
    case 1: return 5;  // Plat -> Avant
    case 5: return 1;  // Avant -> Plat
    case 4: return 3;  // Droite -> Gauche
    case 3: return 4;  // Gauche -> Droite
    case 2: return 6;  // Retourne -> Arriere
    case 6: return 2;  // Arriere -> Retourne
  }

  return orientation;
}

void resetCode() {
  indexCode = 0;
  unlocked = false;

  lcd.setRGB(255, 165, 0);
  lcd.clear();
  lcd.print("Entrez le code");
}

void showEnteredLetters() {

  lcd.setCursor(0, 1);
  lcd.print("                ");  // efface la ligne

  lcd.setCursor(0, 1);
  lcd.print("Code: ");

  for (int i = 0; i < indexCode; i++) {
    int idx = enteredCode[i] - 1;
    lcd.print(letters[idx]);
    lcd.print(" ");
  }

  for (int i = indexCode; i < CODE_LENGTH; i++) {
    lcd.print("_ ");
  }
}

void scrollMessage() {
  if (!unlockedMessage || strlen(unlockedMessage) == 0) return;

  int len = strlen(unlockedMessage);
  if (len == 0) return;

  if (millis() - lastScrollTime >= SCROLL_INTERVAL) {
    lastScrollTime = millis();

    char buffer[17];

    // Ligne 1 : caractères 0 à 15
    for (int i = 0; i < 16; i++) {
      buffer[i] = unlockedMessage[(scrollPos + i) % len];
    }
    buffer[16] = '\0';
    lcd.setCursor(0, 0);
    lcd.print(buffer);

    // Ligne 2 : caractères 16 à 31 (suite du texte)
    for (int i = 0; i < 16; i++) {
      buffer[i] = unlockedMessage[(scrollPos + 16 + i) % len];
    }
    buffer[16] = '\0';
    lcd.setCursor(0, 1);
    lcd.print(buffer);

    scrollPos = (scrollPos + 1) % len;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  pinMode(RESET_LED_PIN, OUTPUT);
  digitalWrite(RESET_LED_PIN, HIGH);
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);

  lcd.begin(16, 2);

  if (myIMU.begin() != 0) {
    lcd.print("IMU ERROR");
    while (1)
      ;
  }

  resetCode();
}

void loop() {
  int resetState = digitalRead(RESET_BUTTON_PIN);

  if (resetState == LOW) {
    Serial.println("RESET MANUEL");

    digitalWrite(RESET_LED_PIN, LOW);

    lcd.setRGB(255, 0, 0);
    lcd.clear();
    lcd.print("Reset...");

    delay(500);

    digitalWrite(RESET_LED_PIN, HIGH);
    resetCode();
  }

  if (unlocked) {
    scrollMessage();
    return;
  }

  float ax = myIMU.readFloatAccelX();
  float ay = myIMU.readFloatAccelY();
  float az = myIMU.readFloatAccelZ();

  int orientation = getOrientation(ax, ay, az);
  int idx = orientation - 1;

  lcd.setRGB(couleurs[idx][0], couleurs[idx][1], couleurs[idx][2]);
  lcd.setCursor(0, 0);
  lcd.print(labels[idx]);
  lcd.print("                 ");

  showEnteredLetters();

  int buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == LOW && lastButtonState == HIGH) {
    Serial.println("Bouton presse");

    enteredCode[indexCode] = orientation;
    indexCode++;

    digitalWrite(LED_PIN, HIGH);
    delay(150);
    digitalWrite(LED_PIN, LOW);

    if (indexCode == CODE_LENGTH) {
      Serial.println("Verification code...");

      bool found = false;
      for (int s = 0; s < NUM_SECRETS; s++) {
        bool match = true;
        for (int i = 0; i < CODE_LENGTH; i++) {
          if (enteredCode[i] != secrets[s].code[i]) {
            match = false;
            break;
          }
        }

        if (match) {
          found = true;
          unlocked = true;

          lcd.setRGB(0, 255, 0);
          lcd.clear();

          unlockedMessage = secrets[s].message;
          scrollPos = 0;

          Serial.print("Code valide : ");
          Serial.println(secrets[s].message);
          break;
        }
      }

      if (!found) {
        lcd.setRGB(255, 0, 0);
        lcd.clear();
        lcd.print("Mauvais code");
        Serial.println("Code incorrect");
        delay(2000);
        resetCode();
      }
    }
  }

  lastButtonState = buttonState;
  delay(120);
}