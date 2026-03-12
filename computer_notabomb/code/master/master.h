#ifndef MASTER_H
#define MASTER_H

#include <Wire.h>
#include <rgb_lcd.h>

rgb_lcd lcd;

#define NB_SEQ 0    // nombre de séquences définies (mettre à jour à chaque nouvelle séquence)
#define DEBOUNCE 20 // mécanique anti-rebond (20ms)
#define LONG_PRESS 1000

// Macros de boutons
#define BUTTONS X(B, 1, 7, 6) X(B, 2, 3, 2) X(R, 1, 9, 8) X(R, 2, 5, 4)
#define X(c, n, p, l)          \
    const uint8_t B##c##n = p; \
    const uint8_t L##c##n = l;
BUTTONS
#undef X
#define X(c, n, p, l) p,
const uint8_t pins[] = {BUTTONS};
#undef X
#define X(c, n, p, l) l,
const uint8_t leds[] = {BUTTONS};
#undef X
const char *names[] = {"b1", "b2", "r1", "r2"};
#define NUM_BTN 4

// Sequences
enum SequenceType
{
    SHORT_INPUT,
    PRESS_INPUT,
    RELEASE_INPUT
};
struct SequenceEvent
{
    SequenceType type;
    uint8_t pin;
};

// Fonctions
// Boutons
void readButtons();
bool anyHoldActive();
void addEvent(SequenceType t, uint8_t pin);
// Séquences
void shortPress(uint8_t i);
void longPress(uint8_t i);
void releaseHold(uint8_t i);
char (*seqPrint(SequenceEvent* seq))[17];
// LCD
void lcdShow(uint8_t line, const char *text);
// Transmission
void sendSeq(uint8_t id);
void sendBuzz(uint8_t id);
void sendMsg1(const char *msg);
void sendMsg2(const char *msg);
void sendCol(uint8_t col);
void sendOpen();
void sendClose();

// États
bool raw[NUM_BTN] = {HIGH, HIGH, HIGH, HIGH};
bool db[NUM_BTN] = {HIGH, HIGH, HIGH, HIGH};
unsigned long tdb[NUM_BTN] = {0};
unsigned long tpress[NUM_BTN] = {0};
bool longSent[NUM_BTN] = {false, false, false, false};
int8_t hold = -1;

SequenceEvent inputSeq[20];
uint8_t inputLen = 0;
uint8_t seqIndex = 0;
uint8_t seqLed = 1;
unsigned long t0 = 0;

// Variables globales
const SequenceEvent *sequences[] = {};
const uint8_t seqLen[] = {};

// Macros séquences
#define S(p) {SHORT_INPUT, p}
#define P(p) {PRESS_INPUT, p}
#define R(p) {RELEASE_INPUT, p}

#endif // MASTER_H