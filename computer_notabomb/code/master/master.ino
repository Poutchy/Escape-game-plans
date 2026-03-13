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
    t0 = millis();
  }
}

void shortPress(uint8_t i){ Serial.print("[BTN] short: "); Serial.println(names[i]); addEvent(SHORT_INPUT,pins[i]); }
void longPress(uint8_t i){ Serial.print("[BTN] long:  "); Serial.println(names[i]); hold=i; addEvent(PRESS_INPUT,pins[i]); }
void releaseHold(uint8_t i){ Serial.print("[BTN] release: "); Serial.println(names[i]); addEvent(RELEASE_INPUT,pins[i]); hold=-1; }

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
  static char lines[2][17];
  lines[0][0] = '\0';
  lines[1][0] = '\0';

  char line1[17] = "";
  char line2[17] = "";

  uint8_t maxEvents = (inputLen > 10) ? 10 : inputLen;

  for(uint8_t i=0;i<maxEvents;i++){
    const char* label = "?";
    for(uint8_t j=0;j<NUM_BTN;j++){
      if(pins[j] == seq[i].pin){ label = names[j]; break; }
    }
    char token[8];
    if(seq[i].type == SHORT_INPUT) {
      snprintf(token,sizeof(token),"%s ",label);
    }
    else if(seq[i].type == PRESS_INPUT)  {
      snprintf(token,sizeof(token),"%s:",label);
    }
    else {
      snprintf(token,sizeof(token),":%s",label);
    }

    if(i < 5) {
      strncat(line1, token, sizeof(line1)-strlen(line1)-1);
    }
    else {
      strncat(line2, token, sizeof(line2)-strlen(line2)-1);
    }
  }

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

/* ---------------- LORA SEND (non-bloquant) ---------------- */
void loraSend(uint8_t* payload, uint8_t size){
  LoRa_Serial.print("AT+MSGHEX=\"");
  for(uint8_t i=0; i<size; i++){
    if(payload[i] < 0x10) LoRa_Serial.print("0");
    LoRa_Serial.print(payload[i], HEX);
  }
  LoRa_Serial.println("\"");
}

/* ---------------- TRANSMISSION ---------------- */
void sendSeq(uint8_t id){ 
  Serial.print("[TX] leds:"); 
  Serial.println(id); 
  slaveSerial.print("leds:"); slaveSerial.println(id); 
}
void sendBuzz(uint8_t id){ 
  Serial.print("[TX] buzz:"); 
  Serial.println(id); 
  slaveSerial.print("buzz:"); 
  slaveSerial.println(id); 
}
void sendMsg1(const char* msg){ 
  Serial.print("[TX] msg1:"); 
  Serial.println(msg); 
  slaveSerial.print("msg1:"); 
  slaveSerial.println(msg); 
}
void sendMsg2(const char* msg){ 
  Serial.print("[TX] msg2:"); 
  Serial.println(msg); 
  slaveSerial.print("msg2:"); 
  slaveSerial.println(msg); }
void sendCol(uint8_t col){ 
  Serial.print("[TX] col:"); 
  Serial.println(col); 
  slaveSerial.print("col:"); 
  slaveSerial.println(col); 
}
void sendOpen(){ 
  Serial.println("[TX] open"); 
  slaveSerial.println("open"); 
}
void sendClose(){ 
  Serial.println("[TX] close"); 
  slaveSerial.println("close");
}

/* ---------------- GESTION CARTE RFID ---------------- */
void handleCard(){
  bool ok = (rfid.uid.size == uidWinSize);
  for(byte i=0; i<uidWinSize && ok; i++)
    if(rfid.uid.uidByte[i] != uidWin[i]) ok = false;

  Serial.print("[RFID] carte ");
  Serial.println(ok ? "OK" : "NOK");

  if(ok){
    lcd.setRGB(0, 255, 0);
    lcdShow(1, "BRAVO !");
    lcdShow(2, "Acces autorise");
    sendSeq(4);
    sendBuzz(7);
    sendCol(2);
    sendOpen();
    gameState = WIN;
  } else {
    lcd.setRGB(255, 0, 0);
    lcdShow(1, "Mauvaise carte!");
    lcdShow(2, "Reessayez...");
    sendBuzz(3);
    loraSend(payloadUpNOK, sizeof(payloadUpNOK));
  }
}

/* ------------------- SETUP ------------------- */
void setup(){
  Serial.begin(115200);
  slaveSerial.begin(4800);
  lcd.begin(16,2);
  lcdShow(1, "Systeme");
  Serial.println("[MASTER] boot");
  for(uint8_t i=0;i<NUM_BTN;i++){ pinMode(pins[i],INPUT_PULLUP); pinMode(leds[i],OUTPUT); }

  // RFID init
  SPI.begin();
  rfid.PCD_Init();
  rfid.PCD_SetAntennaGain(rfid.RxGain_max);
  Serial.println("[RFID] ready");

  // LoRaWAN init
  lorae5.setup_hardware(&Debug_Serial, &LoRa_Serial);
  lorae5.setup_lorawan(REGION, ACTIVATION_MODE, CLASS, SPREADING_FACTOR, ADAPTIVE_DR, CONFIRMED, PORT_UP, false, 60000);
  if(ACTIVATION_MODE == OTAA){
    lcdShow(2, "Loading...");
    while(lorae5.join() == false);
    delay(2000);
  }

  sendSeq(1);
  lcdShow(1, "Entrez la");
  { 
    char buf[17]; 
    snprintf(buf, sizeof(buf), "sequence 1/%d", NB_SEQ); lcdShow(2, buf); 
  }
  t0 = millis();
}

/* ------------------- LOOP ------------------- */
void loop(){

  if(gameState == WIN) return;

  /* -------- SEQUENCES -------- */
  if(gameState == WAIT_SEQ){
    readButtons();

    if(inputLen>0 && !anyHoldActive() && millis()-t0>2000){
      if(checkSequence(seqIndex)){
        seqIndex++;
        lcd.setRGB(0, 255, 0);
        lcdShow(2, "Code valide !");
        sendBuzz(4);

        if(seqIndex == NB_SEQ){
          lcdShow(1, "Passez la carte");
          lcdShow(2, "");
          gameState = WAIT_CARD;
          Serial.println("[GAME] toutes les sequences OK -> WAIT_CARD");
        } else {
          sendSeq(seqIndex + 1);
          lcdShow(1, "Entrez la");
          char buf[17];
          snprintf(buf, sizeof(buf), "sequence %d/%d", seqIndex+1, NB_SEQ);
          lcdShow(2, buf);
        }
      } else {
        lcd.setRGB(255, 0, 0);
        lcdShow(1, "Code invalide!");
        lcdShow(2, "Recommencez...");
        sendBuzz(3);
        char (*lines)[17] = seqPrint(inputSeq);
        sendMsg1(lines[0]);
        sendMsg2(lines[1]);
        seqIndex = 0;
        errorTimer = millis();
        gameState = SEQ_ERROR;
        Serial.println("[GAME] sequence NOK -> SEQ_ERROR");
      }

      inputLen = 0;
      t0 = millis();
    }
    else if(inputLen==0){ t0 = millis(); }
  }

  /* -------- ERREUR SÉQUENCE -------- */
  if(gameState == SEQ_ERROR){
    if(millis() - errorTimer >= 5000){
      lcd.setRGB(255, 255, 255);
      sendSeq(1);
      lcdShow(1, "Entrez la");
      { 
        char buf[17]; 
        snprintf(buf, sizeof(buf), "sequence 1/%d", NB_SEQ); lcdShow(2, buf); 
      }
      gameState = WAIT_SEQ;
      Serial.println("[GAME] reset -> WAIT_SEQ");
    }
  }

  /* -------- PHASE CARTE -------- */
  if(gameState == WAIT_CARD){
    if(rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()){
      handleCard();
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    }
  }
}
