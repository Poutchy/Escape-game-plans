#include <Wire.h>
#include "rgb_lcd.h"

rgb_lcd lcd;

const int potPin = A0;
const int buzzer = A3;
const char code[] = "2418";

int targets[3] = {35, 64, 27};
int tolerance = 0;

int level = 0;
unsigned long startTime = 0;
bool inRange = false;
bool finished = false;
int lastFreq = -1;

int readFrequency() {
  return map(analogRead(potPin), 0, 1023, 0, 100);
}

void displayGame(int freq) {
  lcd.setCursor(0,0);
  lcd.print(level + 1);
  lcd.print("/3 Set freq");

  if (freq != lastFreq) {
    lcd.setCursor(0,1);
    lcd.print("Freq: ");
    lcd.print(freq);
    lcd.print(" Hz  ");
    lastFreq = freq;
  }
}

void displayCode() {
  lcd.setCursor(0,0);
  lcd.print("Decypher done ! ");
  lcd.setCursor(0,1);
  lcd.print("Code: ");
  lcd.print(code);
  lcd.print("      ");
}

bool isCorrect(int freq) {
  return abs(freq - targets[level]) <= tolerance;
}

void nextLevel() {
  for (int i=0; i<3; i++) {
    lcd.setRGB(0,255,0);
    tone(buzzer,31,50);
    delay(50);
    lcd.setRGB(255,165,0);
    delay(50);
  }
  if (level < 2) level++;
  else finished = true;
}

void setup() {
  lcd.begin(16,2);
  lcd.setRGB(255,165,0);
  pinMode(buzzer, OUTPUT);
}

void loop() {
  if (finished) {
    lcd.setRGB(0,255,0);
    displayCode();
    return;
  }

  int freq = readFrequency();
  displayGame(freq);

  if (isCorrect(freq)) {
    if (!inRange) startTime = millis(), inRange = true;

    if (inRange && millis() - startTime >= 3000) {
      nextLevel();
      inRange = false;
      lastFreq = -1;
    }
  } else inRange = false;

  delay(50);
}