#ifndef SLAVE_H
#define SLAVE_H

#include "rgb_lcd.h"
#include <Servo.h>

// Broches
#define BUZZER 2
#define LR 5
#define LY 6
#define LG 7
#define LB 8
#define LOCK A0

// Constantes numériques
#define NB_LED 4

// Délais (en ms)
#define LOCK_CLOSED 100
#define LOCK_OPEN 10
#define BLINK_INTERVAL 500

// Buzzer
#define MAX_MELODY 10

struct Note
{
  int freq;     // Hz — 0 = silence/pause
  int duration; // ms
};

const Note MELODY_POWER_ON[] = {{659, 120}, {784, 120}, {1047, 180}};
const Note MELODY_POWER_OFF[] = {{1047, 120}, {784, 120}, {659, 180}};
const Note MELODY_SUCCESS[] = {{880, 120}, {0, 40}, {1175, 160}};
const Note MELODY_ERROR[] = {{200, 300}, {0, 80}, {200, 300}};
const Note MELODY_WARNING[] = {{700, 200}, {0, 100}, {500, 200}};
const Note MELODY_NOTIFY[] = {{1200, 80}};
const Note MELODY_OPEN[] = {{523, 120}, {659, 120}, {784, 200}};

// Module LED
const int LEDS[] = {LR, LY, LG, LB};
enum LedState
{
  ON,
  OFF,
  BLINK
};

// Fonctions
void setLedPattern(LedState r, LedState y, LedState g, LedState b);
void updateLeds();
void startBuzzer(const Note seq[], int len);
void updateBuzzer();
void key_open();
void key_close();
void handleMsg(int line, const char *text);
void handleColor(int id);
void handleBuzzer(int id);
void handleLed(int id);
void handleCommand(char *c);
void handleSerial();

#endif // SLAVE_H