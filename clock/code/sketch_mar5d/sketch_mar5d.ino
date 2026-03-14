#include "rgb_lcd.h"
#include <Chrono.h>

const unsigned long totalTime = 3600000;
unsigned long elapsed;
char timeString[9];
int ready = 0;
int reset = 0;
int error = 0;
int stop = 0;
int start = 0;

unsigned long penalityTime = 0;

Chrono myChrono;
rgb_lcd lcd;

const int READYSWITCH = D2;
const int READYSLAVE = 2;
const int STARTBUTTON = D3;
const int STOPBUTTON = D4;
const int STOPSLAVE = 4;
const int RESETBUTTON = D5;
const int PENALITYBUTTON = D6;
const int PENALITYSLAVE = 7;
const int BUZZER = D8;


bool isStopped = false;       // état du chrono
unsigned long stopTime = 0;   // temps écoulé quand le chrono est arrêté

void print();
void handleStart();
void handleStop();
void handleReset();
void handlePenality();
void playBuzzer(int f, int t);
void smartDelay(unsigned long ms);

void setup() {
  lcd.begin(16,2);
  myChrono.restart();
  pinMode(READYSWITCH, INPUT);
  pinMode(READYSLAVE, INPUT);
  pinMode(STARTBUTTON, INPUT);
  pinMode(STOPBUTTON, INPUT);
  pinMode(STOPSLAVE, INPUT);
  pinMode(RESETBUTTON, INPUT);
  pinMode(PENALITYBUTTON, INPUT);
  pinMode(PENALITYSLAVE, INPUT);
  pinMode(BUZZER, OUTPUT);
}

void playBuzzer(int f, int t){
  tone(BUZZER, f, t);
}

void loop() {
  if (!ready) {
    ready = digitalRead(READYSWITCH) | digitalRead(READYSLAVE);
    return;
  }

  print();
  if (!start) {
    start = digitalRead(STARTBUTTON);
    if (!start)
      return;
    handleStart();
  }
  if (!isStopped)
  {
    reset = digitalRead(RESETBUTTON);
    stop = digitalRead(STOPBUTTON) | digitalRead(STOPSLAVE);
    error = digitalRead(PENALITYBUTTON) | digitalRead(PENALITYSLAVE);

    handleReset();
    handlePenality();
    handleStop();
  }
  else
  {
    reset = digitalRead(RESETBUTTON);
    handleReset();

    return;
  }


  smartDelay(1000);
}

void print() {
  lcd.setCursor(0,0);

  // Calcul du temps écoulé
  unsigned long currentElapsed = (isStopped ? stopTime : myChrono.elapsed()) + penalityTime;

  // Tant que le chrono n'a pas encore été lancé, remaining vaut totalTime
  unsigned long remaining = (start == 0) ? totalTime : (currentElapsed >= totalTime ? 0 : totalTime - currentElapsed);

  int hours = remaining / 3600000;
  int minutes = (remaining % 3600000) / 60000;
  int seconds = (remaining % 60000) / 1000;

  sprintf(timeString, "%02d:%02d:%02d", hours, minutes, seconds);
  lcd.print(timeString);
  lcd.print(" ");
}

void handleStart() {
  myChrono.restart();
  start = 1;
  stop = 0;
  stopTime = 0;
}

void handleReset() {
  if (reset == HIGH) {
    myChrono.restart();
    lcd.setRGB(255,255,255);
    penalityTime = 0;
    isStopped = false;
    stopTime = 0;
    start = 0;
    stop = 0;
    lcd.clear();
  }
}

void handleStop() {
  if (stop == HIGH) {
    lcd.setRGB(00,255,00);
    stopTime = myChrono.elapsed();
    isStopped = true;
    playBuzzer(880, 120);
    delay(160);
    playBuzzer(1175,160);
  }
}

void handlePenality() {
  if (error) {
    penalityTime += 60000;
    lcd.setRGB(255,00,00);
    playBuzzer(200, 200);
    delay(280);
    playBuzzer(200, 300);
    delay(200);
    lcd.setRGB(255,255,255);
  }
}

void smartDelay(unsigned long ms) {
  unsigned long start = millis();

  while (millis() - start < ms) {

    reset = digitalRead(RESETBUTTON);
    stop = digitalRead(STOPBUTTON) | digitalRead(STOPSLAVE);
    error = digitalRead(PENALITYBUTTON) | digitalRead(PENALITYSLAVE);

    handleReset();
    handlePenality();
    handleStop();

    delay(5);
  }
}