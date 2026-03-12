#ifndef MASTER_H
#define MASTER_H

#include <Wire.h>
#include <rgb_lcd.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>
#include "lorae5.h"
#include "config_application.h"
#include "config_board.h"

// RFID
#define SS_PIN  10
#define RST_PIN A2
MFRC522 rfid(SS_PIN, RST_PIN);

// UID de la carte gagnante
byte uidWin[]   = {0x1F, 0x2D, 0xDF, 0x02};
byte uidWinSize = 4;

rgb_lcd lcd;

#define NB_SEQ 3
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

// État du jeu
enum GameState { WAIT_SEQ, SEQ_ERROR, WAIT_CARD, WIN };
GameState gameState = WAIT_SEQ;

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

void readButtons();
bool anyHoldActive();
void addEvent(SequenceType t, uint8_t pin);
void shortPress(uint8_t i);
void longPress(uint8_t i);
void releaseHold(uint8_t i);
char (*seqPrint(SequenceEvent* seq))[17];
void lcdShow(uint8_t line, const char *text);
void loraSend(uint8_t* payload, uint8_t size);
void sendSeq(uint8_t id);
void sendBuzz(uint8_t id);
void sendMsg1(const char *msg);
void sendMsg2(const char *msg);
void sendCol(uint8_t col);
void sendOpen();
void sendClose();
void handleCard();

// États boutons
bool raw[NUM_BTN] = {HIGH, HIGH, HIGH, HIGH};
bool db[NUM_BTN] = {HIGH, HIGH, HIGH, HIGH};
unsigned long tdb[NUM_BTN] = {0};
unsigned long tpress[NUM_BTN] = {0};
bool longSent[NUM_BTN] = {false, false, false, false};
int8_t hold = -1;

SequenceEvent inputSeq[20];
uint8_t inputLen = 0;
uint8_t seqIndex = 0;
unsigned long t0 = 0;
unsigned long errorTimer = 0;

// Macros séquences
#define S(p) {SHORT_INPUT, p}
#define P(p) {PRESS_INPUT, p}
#define R(p) {RELEASE_INPUT, p}

/* ------------------------------------------------------------------
 * SEQUENCES DU JEU
 *  - S : appui court
 *  - P : maintien long (>= LONG_PRESS)
 *  - R : relâchement d'un maintien
 *
 *  Seq 1 : R2 → R1 → R2 → B2
 *  Seq 2 : B2 → B1 → B1 → R1
 *  Seq 3 : R1 → R2 → R1 → R2 → R2
 * ------------------------------------------------------------------ */
SequenceEvent seq1[] = { S(BR2), S(BR1), S(BR2), S(BB2) };
SequenceEvent seq2[] = { S(BB2), S(BB1), S(BB1), S(BR1) };
SequenceEvent seq3[] = { S(BR1), S(BR2), S(BR1), S(BR2), S(BR2) };

const SequenceEvent *sequences[] = { seq1, seq2, seq3 };
const uint8_t seqLen[] = {
  sizeof(seq1)/sizeof(seq1[0]),
  sizeof(seq2)/sizeof(seq2[0]),
  sizeof(seq3)/sizeof(seq3[0])
};

// Slave serial
SoftwareSerial slaveSerial(A0, A1);

// LoRaWAN
LORAE5 lorae5(devEUI, appEUI, appKey, devAddr, nwkSKey, appSKey);
uint8_t payloadUpNOK[1]   = {0x01};
uint8_t payloadUpALIVE[1] = {0x02};

#endif // MASTER_H