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
    DEBUG_SEQ,
    CODE_RESOLU,
    CODE_MODIFIE_1,
    JOURNAUX_2,
    LISTE_MOTS,
    CODE_RESOLU2,
    CODE_MODIFIE_2,
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
#define MANUAL_MODE S(BB1), S(BB2), S(BR2)
#define JOURNAL S(BR1), S(BB2), S(BR2)
#define MODULE_LED P(BR2), S(BB1), S(BR1), S(BB1), R(BR2)
#define MODIFY_CODE_1 S(BB1), P(BB2), S(BR2), S(BR2), S(BR2), R(BB2), S(BR2)
#define MODULE_WORD P(BR2), S(BB2), S(BB2), S(BB1), R(BR2)
#define MODIFY_CODE_2 S(BB1), P(BB2), S(BB1), S(BR1), S(BB1), R(BB2), S(BR2)
#define REBOOT P(BB1), S(BB2), S(BB2), S(BB2), R(BB1) 
#define PASSWORD S(BB1), S(BB2), S(BR1), S(BR2)

SequenceEvent seqA[] = { MANUAL_MODE };      // ERREUR_CRITIQUE → MODE_MANUAL
SequenceEvent seqB[] = { JOURNAL };          // MODE_MANUAL     → JOURNAUX_1
SequenceEvent seqD[] = { MODULE_LED };       // JOURNAUX_1      → DEBUG_SEQ   → CODE_RESOLU
SequenceEvent seqC[] = { MODIFY_CODE_1 };    // CODE_RESOLU     → CODE_MODIFIE
SequenceEvent seqJ[] = { JOURNAL };          // CODE_MODIFIE    → JOURNAUX_2
SequenceEvent seqE[] = { MODULE_WORD };      // JOURNAUX_2      → LISTE_MOTS → CODE_RESOLU2
SequenceEvent seqK[] = { MODIFY_CODE_2 };    // CODE_RESOLU2    → CODE_MODIFIE_2
SequenceEvent seqF[] = { JOURNAL };          // CODE_MODIFIE_2  → JOURNAUX_3
SequenceEvent seqG[] = { REBOOT };           // JOURNAUX_3      → REDEMARRAGE → LOGIN
SequenceEvent seqI[] = { PASSWORD };         // LOGIN           → WAIT_CARD

/* ------------------------------------------------------------------
 * SÉQUENCES MOTS (5 listes × 5 mots)
 *
 * Liste 0 : Disquette  Windows  Cable     Logiciel  Atari
 * Liste 1 : Dossier    Pixel    Bureau    BIOS      Fichier
 * Liste 2 : Atari      Pixel    Cathodique Fichier  Disquette
 * Liste 3 : Windows    Sega     Modem     Disquette Joystick
 * Liste 4 : Windows    BIOS     Bureau    Dossier   Cable
 * ------------------------------------------------------------------ */
SequenceEvent wSeq_0[] = { S(BB1), S(BB2), S(BB1), S(BR1), S(BR2), S(BR1), S(BR1)};
SequenceEvent wSeq_1[] = { S(BB2), S(BB2), S(BB2), S(BB1), S(BR2)};
SequenceEvent wSeq_2[] = { S(BB2), S(BR2), S(BR2), S(BR2), S(BB1)};
SequenceEvent wSeq_3[] = { S(BB2), S(BB1), S(BB2), S(BB1), S(BR1), S(BB2), S(BB2)};
SequenceEvent wSeq_4[] = { S(BB2), S(BB1), S(BB1), S(BB1), S(BB2), S(BB1), S(BR1)};

const SequenceEvent* wordSeqs[5] = {wSeq_0, wSeq_1, wSeq_2, wSeq_3, wSeq_4};
const uint8_t wordSeqLen[5] = {
  sizeof(wSeq_0)/sizeof(wSeq_0[0]),
  sizeof(wSeq_1)/sizeof(wSeq_1[0]),
  sizeof(wSeq_2)/sizeof(wSeq_2[0]),
  sizeof(wSeq_3)/sizeof(wSeq_3[0]),
  sizeof(wSeq_4)/sizeof(wSeq_4[0])
};

// LISTE_MOTS progress
uint8_t currentWord = 0;

// Codes d'erreur aléatoires (générés à ERREUR_CRITIQUE)
uint16_t errorX = 0, errorY = 0;

// Slave serial
SoftwareSerial slaveSerial(A0, A1);

// LoRaWAN
LORAE5 lorae5(devEUI, appEUI, appKey, devAddr, nwkSKey, appSKey);
uint8_t payloadUpNOK[1] = {0x01};

#endif // MASTER_H
