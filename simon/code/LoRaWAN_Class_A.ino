#include <Arduino.h>
#include "lorae5.h"
#include "config_application.h"
#include "config_board.h"
#include <Servo.h>
#include "rgb_lcd.h"

// ===============================
// Configuration
// ===============================
const int BUZZER_PIN    = A0;
const int RESET_PIN     = D2;
const int SERVO_PIN     = A1; 

const int LED_PINS[]    = {4, 6, 8};
const int BUTTON_PINS[] = {5, 7, 9};
const int FREQUENCIES[] = {262, 330, 392}; // Do, Mi, Sol

const int BUTTON_COUNT  = 3;
const int MAX_SEQUENCE  = 1;

const int NOTE_DURATION = 500;
const int NOTE_PAUSE    = 200;
const int INPUT_FEEDBACK_DURATION = 300;

Servo diskServo;
rgb_lcd lcd;


// ===============================
// LORA PAYLOADS
// ===============================
uint8_t sizePayloadUp = 4;
uint8_t sizePayloadDown = 0;

uint8_t payloadUp[20] = {0xEE, 0x01, 0x00, 0x00}; 
uint8_t payloadDown[20]  = {0};

LORAE5 lorae5(devEUI, appEUI, appKey, devAddr, nwkSKey, appSKey);



// ===============================
// Game state
// ===============================
int sequence[MAX_SEQUENCE];
int sequenceLength = 0;

void processDownlink(); 



// ===============================
// Hardware helpers
// ===============================
void updateLCD(String line1, String line2, int r, int g, int b) {
  lcd.clear();
  lcd.setRGB(r, g, b);
  
  lcd.setCursor(0, 0);
  lcd.print(line1);
  
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void setAllLEDs(int state) {
  for (int i = 0; i < BUTTON_COUNT; i++) {
    digitalWrite(LED_PINS[i], state);
  }
}

void playTone(int index, int duration) {
  digitalWrite(LED_PINS[index], HIGH);
  tone(BUZZER_PIN, FREQUENCIES[index]);

  delay(duration);

  digitalWrite(LED_PINS[index], LOW);
  noTone(BUZZER_PIN);
}



// ===============================
// Game logic
// ===============================
void addRandomStep() {
  if (sequenceLength >= MAX_SEQUENCE) return;

  sequence[sequenceLength] = random(BUTTON_COUNT);
  sequenceLength++;
}

void playSequence() {
  for (int i = 0; i < sequenceLength; i++) {
    playTone(sequence[i], NOTE_DURATION);
    delay(NOTE_PAUSE);
  }
}

int waitForButtonPress() {
  while (true) {
    if (digitalRead(RESET_PIN) == HIGH) {
      return -1;
    }

    for (int i = 0; i < BUTTON_COUNT; i++) {
      if (digitalRead(BUTTON_PINS[i]) == LOW) {
        playTone(i, INPUT_FEEDBACK_DURATION);
        delay(100);
        return i;
      }
    }
  }
}

int checkPlayerInput() {
  updateLCD("ENTER SEQUENCE", "LVL: " + String(sequenceLength) + "/" + String(MAX_SEQUENCE), 255, 100, 0);
  for (int i = 0; i < sequenceLength; i++) {
    int pressed = waitForButtonPress();

    if (pressed == -1) {
      return -1;
    }
    if (pressed != sequence[i]) {
      return 0;
    }
  }

  return 1;
}

void startupSignal() {
  setAllLEDs(HIGH);
  tone(BUZZER_PIN, 200);
  delay(500);
  noTone(BUZZER_PIN);
}

void resetGame() {
  startupSignal();
  sequenceLength = 0;
}

void triggerManualReset() {
  updateLCD("SYSTEM OVERRIDE", "RESTARTING...", 0, 0, 255);
  delay(1500);
  resetGame();
}



// ===============================
// End Logic
// ===============================
void OnSuccess()
{
  Debug_Serial.println("Succes ! Disk release...");
  setAllLEDs(HIGH);
  
  updateLCD("ACCESS GRANTED", "DISK RELEASED", 0, 255, 0);

  diskServo.write(90);
  delay(20000);
  diskServo.write(5);
  
  delay(500);
}

void OnFail()
{
  Debug_Serial.println("ERROR Detected ! sending LoRaWAN payload...");
  setAllLEDs(HIGH);

  updateLCD("ACCESS DENIED!", "SYSTEM LOCKDOWN", 255, 0, 0);
  
  tone(BUZZER_PIN, 150, 1000); 
  
  payloadUp[2] = 0x05;
  
  lorae5.sendData(payloadUp, sizePayloadUp);
  if (lorae5.awaitForDownlinkClass_A(payloadDown, &sizePayloadDown) == RET_DOWNLINK){
    processDownlink();
  }
  
  delay(500);
}



/***********************************************************************/
/* Please see README page on https://github.com/SylvainMontagny/LoRaE5 */
/***********************************************************************/

void setup() 
{
  lcd.begin(16, 2);
  updateLCD("BOOTING OS...", "INITIALIZING HW", 0, 0, 255);
  delay(1000);

  pinMode(RESET_PIN, INPUT_PULLUP);

  // --- INIT BOUTTONS and SERVO ---
  for (int i = 0; i < BUTTON_COUNT; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }
  
  // ALLUMAGE AU BOOT (et pendant la connexion LoRa)
  setAllLEDs(HIGH);
  
  diskServo.attach(SERVO_PIN);
  diskServo.write(5);

  // --- INIT LORA-E5 ---
  updateLCD("CONNECTING TO", "MAINFRAME...", 0, 0, 255);
  lorae5.setup_hardware(&Debug_Serial, &LoRa_Serial);
  lorae5.setup_lorawan(REGION, ACTIVATION_MODE, CLASS, SPREADING_FACTOR, ADAPTIVE_DR, CONFIRMED, PORT_UP, SEND_BY_PUSH_BUTTON, FRAME_DELAY);
  lorae5.printInfo();

  if(ACTIVATION_MODE == OTAA){
    Debug_Serial.println("Join Procedure in progress...");
    while(lorae5.join() == false);
    delay(2000);
  }
  
  randomSeed(analogRead(3));

  updateLCD("SYSTEM READY", "STAND BY", 0, 0, 255);
  startupSignal();
  Debug_Serial.println("System waiting for result...");
}

void loop() 
{  
  if (digitalRead(RESET_PIN) == HIGH) {
    triggerManualReset();
    return;
  }

  addRandomStep();
  
  updateLCD("DECRYPTING...", "OBSERVE PATTERN", 255, 100, 0);
  delay(1000);

  setAllLEDs(LOW);
  playSequence();

  int playerStatus = checkPlayerInput();

  if (playerStatus == -1) {
    triggerManualReset();
    return;
  } 
  else if (playerStatus == 0) {
    OnFail();
    resetGame();
    delay(3000);
    return;
  }

  if (sequenceLength == MAX_SEQUENCE) {
    OnSuccess();
    
    updateLCD("GAME OVER", "PRESS RESET", 255, 255, 255);
    while (digitalRead(RESET_PIN) == HIGH) {
      delay(100);
    }
    triggerManualReset();
  }
  else {
    updateLCD("SEQUENCE OK", "PROCESSING NEXT", 255, 100, 0);
    delay(1500);
  }
}

void processDownlink()
{
  Debug_Serial.println("Message reçu du Game Master (Downlink) !");

  if (sizePayloadDown > 0 && payloadDown[0] == 0xAA) {
      Debug_Serial.println("Ordre de déblocage distant reçu !");
      diskServo.write(90);
      delay(2000);
      diskServo.write(5);
  }
}