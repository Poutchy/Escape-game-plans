#include <Wire.h>
#include <rgb_lcd.h>

rgb_lcd lcd;

/* ------------------- MACROS BOUTONS/LED ------------------- */
#define BUTTONS \
X(B,1,3,2) \
X(B,2,5,4) \
X(R,1,7,6) \
X(R,2,9,8)

#define X(c,n,p,l) const uint8_t B##c##n = p; const uint8_t L##c##n = l;
BUTTONS
#undef X

#define X(c,n,p,l) p,
const uint8_t pins[] = {BUTTONS};
#undef X

#define X(c,n,p,l) l,
const uint8_t leds[] = {BUTTONS};
#undef X

const char* names[] = {"b1","b2","r1","r2"};
#define NUM_BTN 4

/* ------------------- ENUMS / STRUCT ------------------- */
enum SequenceType { SHORT_INPUT, PRESS_INPUT, RELEASE_INPUT };
struct SequenceEvent { SequenceType type; uint8_t pin; };

/* ------------------- SEQUENCES CONFIG ------------------- */
#define S(p) {SHORT_INPUT,p}
#define P(p) {PRESS_INPUT,p}
#define R(p) {RELEASE_INPUT,p}

// exemple : r1 b2 b1:r2 r1 b2 :b1 b2
SequenceEvent seq1[] = {
  S(BR1), S(BB2),
  P(BB1),
  S(BR2), S(BR1), S(BB2),
  R(BB1),
  S(BB2)
};

SequenceEvent seq2[] = {
  S(BB2),
  P(BB1),
  S(BB2), S(BR1), S(BR2),
  R(BB1),
  S(BB2), S(BR1)
};

const SequenceEvent* sequences[] = {seq1, seq2};
const uint8_t seqLen[] = {sizeof(seq1)/sizeof(seq1[0])};
#define NB_SEQ 2

/* ------------------- STATE ------------------- */
bool raw[NUM_BTN]={HIGH,HIGH,HIGH,HIGH};
bool db[NUM_BTN]={HIGH,HIGH,HIGH,HIGH};
unsigned long tdb[NUM_BTN]={0};
unsigned long tpress[NUM_BTN]={0};
bool longSent[NUM_BTN]={false,false,false,false};
int8_t hold=-1;

SequenceEvent inputSeq[20];
uint8_t inputLen=0;
uint8_t seqIndex=0;
uint8_t seqLed=1;

unsigned long t0=0;

/* ------------------- HELPERS ------------------- */
void addEvent(SequenceType t, uint8_t pin){
  if(inputLen < 20){
    inputSeq[inputLen++] = {t, pin};
    t0 = millis(); // reset timeout à chaque événement
  }
}

void resetInput(){ inputLen=0; }

bool checkSequence(uint8_t idx){
  if(inputLen != seqLen[idx]) return false;
  for(uint8_t i=0;i<inputLen;i++){
    if(inputSeq[i].type!=sequences[idx][i].type) return false;
    if(inputSeq[i].pin!=sequences[idx][i].pin) return false;
  }
  return true;
}

void sendSeq(uint8_t v){ seqLed=v; Serial1.print("seq_led:"); Serial1.println(v); }
void sendMsg(const char* msg){ Serial1.println(msg); }
void lcdShow(const char* l1,const char* l2){ lcd.clear(); lcd.setCursor(0,0); lcd.print(l1); lcd.setCursor(0,1); lcd.print(l2); }

/* ------------------- DEBUG PRINT ------------------- */
void printSequenceDebug(){
  for(uint8_t i=0;i<inputLen;i++){
    SequenceEvent e = inputSeq[i];
    switch(e.type){
      case SHORT_INPUT:
        for(uint8_t j=0;j<NUM_BTN;j++) if(pins[j]==e.pin){ Serial.print(names[j]); break; }
        Serial.print(" ");
        break;
      case PRESS_INPUT:
        for(uint8_t j=0;j<NUM_BTN;j++) if(pins[j]==e.pin){ Serial.print(names[j]); break; }
        Serial.print(": ");
        break;
      case RELEASE_INPUT:
        Serial.print(":");
        for(uint8_t j=0;j<NUM_BTN;j++) if(pins[j]==e.pin){ Serial.print(names[j]); break; }
        Serial.print(" ");
        break;
    }
  }
  Serial.println();
}

/* ------------------- EVENTS ------------------- */
void shortPress(uint8_t i){ addEvent(SHORT_INPUT,pins[i]); }
void longPress(uint8_t i){ hold=i; addEvent(PRESS_INPUT,pins[i]); }
void releaseHold(uint8_t i){ addEvent(RELEASE_INPUT,pins[i]); hold=-1; }

/* ------------------- BUTTON READ ------------------- */
#define DEBOUNCE 20
#define LONG_PRESS 1000

void readButtons(){
  unsigned long now = millis();
  for(uint8_t i=0;i<NUM_BTN;i++){
    bool r = digitalRead(pins[i]);
    if(r != raw[i]){ raw[i]=r; tdb[i]=now; }
    if((now-tdb[i])>DEBOUNCE && db[i]!=raw[i]){
      db[i]=raw[i];
      digitalWrite(leds[i], !db[i]);
      if(db[i]==LOW){ tpress[i]=now; longSent[i]=false; }
      else {
        if(hold==i) releaseHold(i);
        else if(!longSent[i]) shortPress(i);
      }
    }
    // Appui long
    if(db[i]==LOW && !longSent[i] && (now-tpress[i])>=LONG_PRESS){
      longSent[i]=true;
      longPress(i);
    }
  }
}

bool anyHoldActive(){
  for(uint8_t i=0;i<NUM_BTN;i++) if(db[i]==LOW) return true;
  return false;
}

/* ------------------- LED DISPLAY ------------------- */
#define LR A0
#define LY A1
#define LG A2
#define LB A3

void showSeq(){
  digitalWrite(LR,LOW); digitalWrite(LY,LOW); digitalWrite(LG,LOW); digitalWrite(LB,LOW);
  switch(seqLed){
    case 1: digitalWrite(LR,HIGH); break;
    case 2: digitalWrite(LY,HIGH); break;
    case 3: digitalWrite(LG,HIGH); break;
    case 4: digitalWrite(LB,HIGH); break;
  }
}

/* ------------------- SETUP ------------------- */
void setup(){
  Serial.begin(115200);
  Serial1.begin(115200);
  lcd.begin(16,2); lcd.setRGB(0,255,0); lcdShow("Systeme","Pret");
  for(uint8_t i=0;i<NUM_BTN;i++){ pinMode(pins[i],INPUT_PULLUP); pinMode(leds[i],OUTPUT); }
  pinMode(LR,OUTPUT); pinMode(LY,OUTPUT); pinMode(LG,OUTPUT); pinMode(LB,OUTPUT);
  sendSeq(seqLed); // démarrer la première séquence LED
  t0 = millis();
}

/* ------------------- LOOP ------------------- */
void loop(){
  readButtons();
  showSeq();

  if(inputLen>0 && !anyHoldActive() && millis()-t0>2000){
    if(checkSequence(seqIndex)){
      sendMsg("Code valide");
      lcd.setRGB(0,255,0); lcdShow("Resultat","Code valide");
      printSequenceDebug(); // debug
      seqIndex++; 
      Serial.print("SEQ INDEX : "); Serial.println(seqIndex);
      Serial.print("NB SEQ : "); Serial.println(NB_SEQ);
      Serial.print("COMPARE : "); Serial.println(seqIndex>NB_SEQ);
      if(seqIndex==NB_SEQ) sendSeq(0); else sendSeq(seqIndex+1);
    } else {
      sendMsg("Code invalide");
      lcd.setRGB(255,0,0); lcdShow("Resultat","Code invalide");
      printSequenceDebug(); // debug
    }
    resetInput();
    t0 = millis();
  }
  else if(inputLen==0){ t0 = millis(); } // reset timer si pas d'input
}