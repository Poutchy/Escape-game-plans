#include "rgb_lcd.h"
#include <Servo.h>

// =============================================================================
// PIN DEFINITIONS
// =============================================================================

#define BUZZER 2

#define LR 5
#define LY 6
#define LG 7
#define LB 8
#define NB_LED 4

#define LOCK A0

// =============================================================================
// CONSTANTS
// =============================================================================

#define LOCK_CLOSED 100
#define LOCK_OPEN 10

const int BLINK_INTERVAL = 500; // ms

// Note type used by the buzzer melody player
struct Note
{
  int freq;     // Hz — 0 = silence/pause
  int duration; // ms
};

// =============================================================================
// LED MODULE
// =============================================================================

const int LEDS[] = {LR, LY, LG, LB};

enum LedState
{
  ON,
  OFF,
  BLINK
};

volatile LedState leds_state[] = {LedState::OFF, LedState::OFF, LedState::OFF, LedState::OFF};
unsigned long lastBlink = 0;
bool blinkState = LOW;

void setLedPattern(LedState r, LedState y, LedState g, LedState b)
{
  leds_state[0] = r;
  leds_state[1] = y;
  leds_state[2] = g;
  leds_state[3] = b;
}

void updateLeds()
{
  if (millis() - lastBlink >= BLINK_INTERVAL)
  {
    lastBlink = millis();
    blinkState = (blinkState == LOW) ? HIGH : LOW;
  }
  for (int i = 0; i < NB_LED; i++)
  {
    switch (leds_state[i])
    {
    case LedState::OFF:
      digitalWrite(LEDS[i], LOW);
      break;
    case LedState::ON:
      digitalWrite(LEDS[i], HIGH);
      break;
    case LedState::BLINK:
      digitalWrite(LEDS[i], blinkState);
      break;
    }
  }
}

// =============================================================================
// BUZZER MODULE
// =============================================================================

#define MAX_MELODY 10

const Note MELODY_POWER_ON[] = {{659, 120}, {784, 120}, {1047, 180}};
const Note MELODY_POWER_OFF[] = {{1047, 120}, {784, 120}, {659, 180}};
const Note MELODY_SUCCESS[] = {{880, 120}, {0, 40}, {1175, 160}};
const Note MELODY_ERROR[] = {{200, 300}, {0, 80}, {200, 300}};
const Note MELODY_WARNING[] = {{700, 200}, {0, 100}, {500, 200}};
const Note MELODY_NOTIFY[] = {{1200, 80}};
const Note MELODY_OPEN[] = {{523, 120}, {659, 120}, {784, 200}};

Note buzzerSequence[MAX_MELODY];
int buzzerLength = 0;
int buzzerIndex = 0;
unsigned long buzzerTimer = 0;
bool buzzerPlaying = false;

void startBuzzer(const Note seq[], int len)
{
  memcpy(buzzerSequence, seq, sizeof(Note) * len);
  buzzerLength = len;
  buzzerIndex = 0;
  int freq = buzzerSequence[0].freq;
  if (freq > 0)
    tone(BUZZER, freq);
  else
    noTone(BUZZER);
  buzzerTimer = millis();
  buzzerPlaying = true;
}

void updateBuzzer()
{
  if (!buzzerPlaying)
    return;
  if (millis() - buzzerTimer >= (unsigned long)buzzerSequence[buzzerIndex].duration)
  {
    buzzerIndex++;
    if (buzzerIndex >= buzzerLength)
    {
      noTone(BUZZER);
      buzzerPlaying = false;
      return;
    }
    int freq = buzzerSequence[buzzerIndex].freq;
    if (freq > 0)
      tone(BUZZER, freq);
    else
      noTone(BUZZER);
    buzzerTimer = millis();
  }
}

// =============================================================================
// SERVO / KEY MODULE
// =============================================================================

Servo lock;

void key_open()
{
  Serial.println("OPENING...");
  lock.write(LOCK_OPEN);
}
void key_close()
{
  Serial.println("CLOSING...");
  lock.write(LOCK_CLOSED);
}

// =============================================================================
// LCD MODULE
// =============================================================================

rgb_lcd lcd;

// =============================================================================
// UART / COMMAND PARSER
// =============================================================================

char cmd[32];

void handleMsg(int line, const char *text)
{
  Serial.print("MESSAGE line ");
  Serial.print(line);
  Serial.print(": ");
  Serial.println(text);
  lcd.setCursor(0, line - 1);
  lcd.print("                "); // clear line
  lcd.setCursor(0, line - 1);
  lcd.print(text);
}

void handleColor(int id)
{
  Serial.print("COLOR ");
  Serial.println(id);
  switch (id)
  {
  case 0: // off
    lcd.setRGB(0, 0, 0);
    break;
  case 1: // red
    lcd.setRGB(255, 0, 0);
    break;
  case 2: // green
    lcd.setRGB(0, 255, 0);
    break;
  case 3: // blue
    lcd.setRGB(0, 0, 255);
    break;
  case 4: // yellow
    lcd.setRGB(255, 255, 0);
    break;
  case 5: // magenta
    lcd.setRGB(255, 0, 255);
    break;
  case 6: // cyan
    lcd.setRGB(0, 255, 255);
    break;
  case 7: // white
    lcd.setRGB(255, 255, 255);
    break;
  default:
    lcd.setRGB(0, 0, 0);
    break;
  }
}

void handleBuzzer(int id)
{
  Serial.print("BUZZER ");
  Serial.println(id);
  switch (id)
  {
  case 1:
    startBuzzer(MELODY_POWER_ON, 3);
    break;
  case 2:
    startBuzzer(MELODY_POWER_OFF, 3);
    break;
  case 3:
    startBuzzer(MELODY_ERROR, 3);
    break;
  case 4:
    startBuzzer(MELODY_SUCCESS, 3);
    break;
  case 5:
    startBuzzer(MELODY_NOTIFY, 1);
    break;
  case 6:
    startBuzzer(MELODY_WARNING, 3);
    break;
  case 7:
    startBuzzer(MELODY_OPEN, 3);
    break;
  }
}

void handleLed(int id)
{
  Serial.print("LED PATTERN ");
  Serial.println(id);
  switch (id)
  {
    case 1:
      setLedPattern(LedState::BLINK, LedState::OFF, LedState::ON, LedState::BLINK);
      break;
    case 2:
      setLedPattern(LedState::ON, LedState::BLINK, LedState::OFF, LedState::OFF);
      break;
    case 3:
      setLedPattern(LedState::OFF, LedState::OFF, LedState::ON, LedState::ON);
      break;
    case 4:
      setLedPattern(LedState::BLINK, LedState::BLINK, LedState::BLINK, LedState::BLINK);
      break;
    default:
      setLedPattern(LedState::OFF, LedState::OFF, LedState::OFF, LedState::OFF);
      break;
  }
}

void handleCommand(char *c)
{
  Serial.print("[SLAVE] received: ");
  Serial.println(c);

  char *sep = strchr(c, ':');
  char *arg = (sep != nullptr) ? sep + 1 : (char *)"";
  int val = (sep != nullptr) ? atoi(sep + 1) : 0;
  if (sep != nullptr)
    *sep = '\0'; // split in-place: c now holds only the name

  if (strcmp(c, "msg1") == 0)
    handleMsg(1, arg);
  else if (strcmp(c, "msg2") == 0)
    handleMsg(2, arg);
  else if (strcmp(c, "col") == 0)
    handleColor(val);
  else if (strcmp(c, "buzz") == 0)
    handleBuzzer(val);
  else if (strcmp(c, "leds") == 0)
    handleLed(val);
  else if (strcmp(c, "open") == 0)
    key_open();
  else if (strcmp(c, "close") == 0)
    key_close();
  else
  {
    Serial.println("Unknown command:");
    Serial.println(c);
  }
}

void handleSerial()
{
  if (Serial1.available())
  {
    int len = Serial1.readBytesUntil('\n', cmd, sizeof(cmd) - 1);
    cmd[len] = '\0';
    if (len > 0 && cmd[len - 1] == '\r')
      cmd[len - 1] = '\0'; // strip CR if present
    handleCommand(cmd);
  }
}

// =============================================================================
// SETUP
// =============================================================================

void setup()
{
  Serial.begin(9600);    // debug
  Serial1.begin(4800); // UART

  lcd.begin(16, 2);

  lock.attach(LOCK);

  pinMode(BUZZER, OUTPUT);
  pinMode(LR, OUTPUT);
  pinMode(LY, OUTPUT);
  pinMode(LG, OUTPUT);
  pinMode(LB, OUTPUT);

  lock.write(LOCK_CLOSED);

  Serial.println("[SLAVE] ready");
}

// =============================================================================
// MAIN LOOP
// =============================================================================

void loop()
{
  handleSerial();
  updateLeds();
  updateBuzzer();
}