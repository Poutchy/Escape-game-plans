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

#define NB_SEQ           5
#define DEBOUNCE         20     // mécanique anti-rebond (20ms)
#define LONG_PRESS       1000
#define DEBUG_BAUD       115200
#define SLAVE_BAUD       4800
#define SEQ_TIMEOUT_MS   2000   // idle window to validate a sequence
#define ERROR_DISPLAY_MS 3000   // how long the error screen stays
#define INPUT_MAX        20     // max events in a sequence
#define LCD_COLS         16
#define LCD_ROWS         2
#define JOIN_TIMEOUT_MS  60000  // max time to wait for LoRaWAN OTAA join

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
enum GameState {
    ERREUR_CRITIQUE,
    MODE_MANUAL,
    JOURNAUX_1,
    JOURNAUX_1b,
    DEBUG_SEQ,
    JOURNAUX_2,
    LISTE_MOTS,
    CODE_RESOLU,
    JOURNAUX_3,
    REDEMARRAGE,
    LOGIN,
    WAIT_CARD,
    SEQ_ERROR,
    WIN
};
GameState gameState = ERREUR_CRITIQUE;
GameState prevState = ERREUR_CRITIQUE;  // for SEQ_ERROR recovery

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

// Forward declarations
void readButtons();
bool anyHoldActive();
void addEvent(SequenceType t, uint8_t pin);
void shortPress(uint8_t i);
void longPress(uint8_t i);
void releaseHold(uint8_t i);
char (*seqPrint(SequenceEvent* seq))[17];
void lcdShow(uint8_t line, const char *text);
void loraSend(uint8_t* payload, uint8_t size);
void sendCmd(const char* key, const char* val = nullptr);
void sendCmd(const char* key, int val);
void handleCard();
bool matchSeq(const SequenceEvent* expected, uint8_t len);
bool seqReady();
void enterState(GameState s);
void enterError(GameState returnTo);
void exitError();

// États boutons
bool rawState[NUM_BTN]              = {HIGH, HIGH, HIGH, HIGH};
bool buttonState[NUM_BTN]           = {HIGH, HIGH, HIGH, HIGH};
unsigned long debounceTime[NUM_BTN] = {0};
unsigned long pressTime[NUM_BTN]    = {0};
bool longTriggered[NUM_BTN]         = {false, false, false, false};
int8_t holdButton = -1;

SequenceEvent inputSeq[INPUT_MAX];
uint8_t inputCount       = 0;
uint8_t currentSeq       = 0;
unsigned long inputWindowStart  = 0;
unsigned long errorDisplayStart = 0;

// Macros séquences
#define S(p) {SHORT_INPUT, p}
#define P(p) {PRESS_INPUT, p}
#define R(p) {RELEASE_INPUT, p}

/* ------------------------------------------------------------------
 * SEQUENCES DEBUG (seq1–seq5)
 *  Seq 1 : R2 → R1 → R2 → B2
 *  Seq 2 : B2 → B1 → B1 → R1
 *  Seq 3 : B1 → B2 → R1 → R2 → R2 → B1 → B1
 *  Seq 4 : B1 → R2
 *  Seq 5 : R1 → R2 → R1 → R2 → R2
 * ------------------------------------------------------------------ */
SequenceEvent seq1[] = { S(BR2), S(BR1), S(BR2), S(BB2) };
SequenceEvent seq2[] = { S(BB2), S(BB1), S(BB1), S(BR1) };
SequenceEvent seq3[] = { S(BB1), S(BB2), S(BR1), S(BR2), S(BR2), S(BB1), S(BB1) };
SequenceEvent seq4[] = { S(BB1), S(BR2) };
SequenceEvent seq5[] = { S(BR1), S(BR2), S(BR1), S(BR2), S(BR2) };

const SequenceEvent *sequences[] = { seq1, seq2, seq3, seq4, seq5 };
const uint8_t seqLen[] = {
  sizeof(seq1)/sizeof(seq1[0]),
  sizeof(seq2)/sizeof(seq2[0]),
  sizeof(seq3)/sizeof(seq3[0]),
  sizeof(seq4)/sizeof(seq4[0]),
  sizeof(seq5)/sizeof(seq5[0])
};

/* ------------------------------------------------------------------
 * SEQUENCES D'ÉTAPES
 * Placeholder = S(BB1), S(BB2), S(BB1) — modifier individuellement
 * ------------------------------------------------------------------ */
#define PLACEHOLDER_SEQ S(BB1), S(BB2), S(BB1)

SequenceEvent seqA[] = { PLACEHOLDER_SEQ };  // ERREUR_CRITIQUE → MODE_MANUAL
SequenceEvent seqB[] = { PLACEHOLDER_SEQ };  // MODE_MANUAL    → JOURNAUX_1
SequenceEvent seqC[] = { PLACEHOLDER_SEQ };  // JOURNAUX_1     → JOURNAUX_1b (clear msg2)
SequenceEvent seqD[] = { PLACEHOLDER_SEQ };  // JOURNAUX_1b    → DEBUG_SEQ   (clear msg1)
SequenceEvent seqE[] = { PLACEHOLDER_SEQ };  // JOURNAUX_2     → LISTE_MOTS
SequenceEvent seqF[] = { PLACEHOLDER_SEQ };  // CODE_RESOLU    → JOURNAUX_3
SequenceEvent seqG[] = { PLACEHOLDER_SEQ };  // JOURNAUX_3     → REDEMARRAGE
SequenceEvent seqH[] = { PLACEHOLDER_SEQ };  // REDEMARRAGE    → LOGIN
SequenceEvent seqI[] = { PLACEHOLDER_SEQ };  // LOGIN          → WAIT_CARD

/* ------------------------------------------------------------------
 * SÉQUENCES MOTS (5 listes × 5 mots)
 *
 * Liste 0 : Disquette  Windows  Cable     Logiciel  Atari
 * Liste 1 : Dossier    Pixel    Bureau    BIOS      Fichier
 * Liste 2 : Atari      Pixel    Cathodique Fichier  Disquette
 * Liste 3 : Windows    Sega     Modem     Disquette Joystick
 * Liste 4 : Windows    BIOS     Bureau    Dossier   Cable
 * ------------------------------------------------------------------ */
SequenceEvent wSeq_0_0[] = { S(BB1) };
SequenceEvent wSeq_0_1[] = { S(BB2), S(BB1) };
SequenceEvent wSeq_0_2[] = { S(BR1) };
SequenceEvent wSeq_0_3[] = { S(BR2) };
SequenceEvent wSeq_0_4[] = { S(BR1), S(BR1) };

SequenceEvent wSeq_1_0[] = { S(BB2) };
SequenceEvent wSeq_1_1[] = { S(BB2) };
SequenceEvent wSeq_1_2[] = { S(BB2) };
SequenceEvent wSeq_1_3[] = { S(BB1) };
SequenceEvent wSeq_1_4[] = { S(BR2) };

SequenceEvent wSeq_2_0[] = { PLACEHOLDER_SEQ };
SequenceEvent wSeq_2_1[] = { S(BB2) };
SequenceEvent wSeq_2_2[] = { S(BR2), S(BR2) };
SequenceEvent wSeq_2_3[] = { S(BR2) };
SequenceEvent wSeq_2_4[] = { S(BB1) };

SequenceEvent wSeq_3_0[] = { S(BB2), S(BB1) };
SequenceEvent wSeq_3_1[] = { S(BB2) };
SequenceEvent wSeq_3_2[] = { S(BB1) };
SequenceEvent wSeq_3_3[] = { S(BR1) };
SequenceEvent wSeq_3_4[] = { S(BB2), S(BB2) };

SequenceEvent wSeq_4_0[] = { S(BB2), S(BB1) };
SequenceEvent wSeq_4_1[] = { S(BB1), S(BB1) };
SequenceEvent wSeq_4_2[] = { S(BB2) };
SequenceEvent wSeq_4_3[] = { S(BB1) };
SequenceEvent wSeq_4_4[] = { S(BR1) };

const SequenceEvent* wordSeqs[5][5] = {
  {wSeq_0_0, wSeq_0_1, wSeq_0_2, wSeq_0_3, wSeq_0_4},
  {wSeq_1_0, wSeq_1_1, wSeq_1_2, wSeq_1_3, wSeq_1_4},
  {wSeq_2_0, wSeq_2_1, wSeq_2_2, wSeq_2_3, wSeq_2_4},
  {wSeq_3_0, wSeq_3_1, wSeq_3_2, wSeq_3_3, wSeq_3_4},
  {wSeq_4_0, wSeq_4_1, wSeq_4_2, wSeq_4_3, wSeq_4_4},
};
#define SL(a) (sizeof(a)/sizeof(a[0]))
const uint8_t wordSeqLen[5][5] = {
  { SL(wSeq_0_0), SL(wSeq_0_1), SL(wSeq_0_2), SL(wSeq_0_3), SL(wSeq_0_4) },
  { SL(wSeq_1_0), SL(wSeq_1_1), SL(wSeq_1_2), SL(wSeq_1_3), SL(wSeq_1_4) },
  { SL(wSeq_2_0), SL(wSeq_2_1), SL(wSeq_2_2), SL(wSeq_2_3), SL(wSeq_2_4) },
  { SL(wSeq_3_0), SL(wSeq_3_1), SL(wSeq_3_2), SL(wSeq_3_3), SL(wSeq_3_4) },
  { SL(wSeq_4_0), SL(wSeq_4_1), SL(wSeq_4_2), SL(wSeq_4_3), SL(wSeq_4_4) },
};
#undef SL

// LISTE_MOTS progress
uint8_t currentList = 0;
uint8_t currentWord = 0;

// Codes d'erreur aléatoires (générés à ERREUR_CRITIQUE)
uint16_t errorX = 0, errorY = 0, errorZ = 0;

// Slave serial
SoftwareSerial slaveSerial(A0, A1);

// LoRaWAN
LORAE5 lorae5(devEUI, appEUI, appKey, devAddr, nwkSKey, appSKey);
uint8_t payloadUpNOK[1]   = {0x01};
uint8_t payloadUpALIVE[1] = {0x02};

#endif // MASTER_H
