#include "master.h"

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

void addEvent(SequenceType t, uint8_t pin){
  if(inputLen < 20){
    inputSeq[inputLen++] = {t, pin};
    t0 = millis(); // reset timeout à chaque événement
  }
}

void shortPress(uint8_t i){ addEvent(SHORT_INPUT,pins[i]); }
void longPress(uint8_t i){ hold=i; addEvent(PRESS_INPUT,pins[i]); }
void releaseHold(uint8_t i){ addEvent(RELEASE_INPUT,pins[i]); hold=-1; }

/* ------------------------------------------------------------------
 * ------------- ENREGISTREMENT DES SÉQUENCES ATTENDUES -------------
 * ------------------------------------------------------------------
 *  Notice
 *  - S : appui court -- pression & relâchement sans dépasser le seuil de LONG_PRESS
 *  - P : pression    -- maintien du bouton pendant au moins LONG_PRESS
 *  - R : relâchement -- relâchement d'un bouton précédemment en pression
 *  
 *  Exemple : S(BR1) = appui court sur le bouton R1
 *            P(BB2) = pression sur le bouton B2
 *
 *  La séquence attendue est définie dans les tableaux seq1, seq2, etc.
 *  Une pression doit être suivi d'un relâchement avant la fin de la séquence pour être valide.
 *  Une séquence ayant un appui court du bouton en pression est invalide.
 *  Exemple : S(BR1), P(BB2), S(BR2), R(BB2), S(BB1) est valide.
 *  Exemple : S(BR1), P(BB2), S(BR2), S(BB1) est invalide (manque le relâchement de BB2).
 *  Exemple : S(BR1), P(BB2), S(BB1), R(BB2), S(BR2) est invalide (BB1 ne peut pas être pressé pendant que BB2 est en appui long).
 *
 *  On ne peut pas avoir 2 appuis longs simultanés dans une séquence valide.
 *  Exemple : S(BR1), P(BB2), P(BR2) est invalide.
 *  Exemple : S(BR1), P(BB2), R(BB2), P(BR2), S(BB2), R(BR2) est valide.
 *
 *  Exemple d'enregistrement de séquence :
 *  SequenceEvent seq_test[] = {
 *    S(BR1), S(BB2),
 *    P(BB1),
 *    S(BR2), S(BR1), S(BB2),
 *    R(BB1),
 *    S(BB2)
 *  };
 * ------------------------------------------------------------------ */
bool checkSequence(uint8_t idx){
  if(inputLen != seqLen[idx]) return false;
  for(uint8_t i=0;i<inputLen;i++){
    if(inputSeq[i].type!=sequences[idx][i].type) return false;
    if(inputSeq[i].pin!=sequences[idx][i].pin) return false;
  }
  return true;
}

char (*seqPrint(SequenceEvent* seq))[17]{
  static char lines[2][17]; // each line 16 chars + null
  lines[0][0] = '\0'; 
  lines[1][0] = '\0';

  char line1[17] = "";
  char line2[17] = "";

  uint8_t maxEvents = (inputLen > 10) ? 10 : inputLen;

  for(uint8_t i=0;i<maxEvents;i++){
    const char* label = "?";

    for(uint8_t j=0;j<NUM_BTN;j++){
      if(pins[j] == seq[i].pin){
        label = names[j];
        break;
      }
    }

    char token[8];

    if(seq[i].type == SHORT_INPUT){
      snprintf(token,sizeof(token),"%s ",label);
    }
    else if(seq[i].type == PRESS_INPUT){
      snprintf(token,sizeof(token),"%s:",label);
    }
    else{ // RELEASE_INPUT
      snprintf(token,sizeof(token),":%s",label);
    }

    if(i < 5) strncat(line1, token, sizeof(line1)-strlen(line1)-1);
    else      strncat(line2, token, sizeof(line2)-strlen(line2)-1);
  }

  // pad or truncate to exactly 16 chars
  size_t l1 = strlen(line1);
  if(l1 >= 16) line1[16] = '\0';
  else for(size_t k=l1;k<16;k++) line1[k] = ' ';
  line1[16] = '\0';

  size_t l2 = strlen(line2);
  if(l2 >= 16) line2[16] = '\0';
  else for(size_t k=l2;k<16;k++) line2[k] = ' ';
  line2[16] = '\0';

  memcpy(lines[0], line1, 17);
  memcpy(lines[1], line2, 17);

  return lines;
}

/* ----------------- LCD DISPLAY ----------------- */
void lcdShow(uint8_t line, const char* text){
  lcd.setCursor(0, line-1);
  lcd.print("                ");
  lcd.setCursor(0, line-1);
  lcd.print(text);
}

/* ---------------- TRANSMISSION ---------------- */
void sendSeq(uint8_t id){ Serial1.print("leds:"); Serial1.println(id); }        // Déclenche la séquence LED #id
void sendBuzz(uint8_t id){ Serial1.print("buzz:"); Serial1.println(id); }       // Déclenche le patterne #id de buzzer
void sendMsg1(const char* msg){ Serial1.print("msg1:"); Serial1.println(msg); } // 1re ligne LCD
void sendMsg2(const char* msg){ Serial1.print("msg2:"); Serial1.println(msg); } // 2de ligne LCD
void sendCol(uint8_t col){ Serial1.print("col:"); Serial1.println(col); }       // Déclenche le preset de couleur #col
void sendOpen(){ Serial1.println("open"); }                                     // Ouvre la serrure
void sendClose(){ Serial1.println("close"); }                                   // Ferme la serrure

/* ------------------- SETUP ------------------- */
void setup(){
  Serial1.begin(115200);
  lcd.begin(16,2);
  lcdShow(1, "Systeme");
  for(uint8_t i=0;i<NUM_BTN;i++){ pinMode(pins[i],INPUT_PULLUP); pinMode(leds[i],OUTPUT); }
  sendSeq(seqLed);
  t0 = millis();
}

/* ------------------- LOOP ------------------- */
void loop(){
  readButtons();

  if(inputLen>0 && !anyHoldActive() && millis()-t0>2000){
    lcdShow(1, "Sequence obtenue");
    if(checkSequence(seqIndex)){
      lcd.setRGB(0,255,0);
      lcdShow(2, "Code valide !");
      seqIndex++;
      if(seqIndex==NB_SEQ) sendSeq(0); else sendSeq(++seqIndex);
    } else {
      lcd.setRGB(255,0,0);
      lcdShow(2, "Code invalide...");
      char (*lines)[17] = seqPrint(inputSeq);
      sendMsg1(lines[0]);
      sendMsg2(lines[1]);
    }
    inputLen=0; // reset input
    t0 = millis();
  }
  else if(inputLen==0){ t0 = millis(); } // reset timer si pas d'input
}