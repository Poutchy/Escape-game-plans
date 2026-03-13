#include "rgb_lcd.h"

#define ADC_REF 5
#define GROVE_VCC 5
#define FULL_ANGLE 360

#define MIN_FREQ 136
#define MAX_FREQ 900

#define ROTARY_ANGLE_SENSOR A0
#define BUZZER 4

#define MARGIN 3              // frequency precision required
#define LOCK_TIME 3000        // must hold frequency for 3s
#define MESSAGE_INTERVAL 5000 // repeat message every 5s

#define NB_FREQ 14

#define MORSE_FREQ 800
#define DOT_TIME 120
#define DASH_TIME 360
#define SYMBOL_SPACE 120
#define LETTER_SPACE 300

rgb_lcd lcd;

/* ---------- STATIONS ---------- */

const int FREQS[NB_FREQ] = {
    149, // red
    200, // sos
    280, // blue
    333, // truth
    404, // ???
    475, // yellow
    538, // code
    567, // white
    611, // dcxi
    666, // devil
    708, // boca
    777, // casino
    806, // beer
    872, // green
};

const char *MESSAGES[NB_FREQ] = {
    ".-. . -..",                // RED
    "... --- ...",              // SOS
    "-... .-.. ..- .",          // BLUE
    "- .-. ..- - ....",         // TRUTH
    "..--.. ..--.. ..--..",     // ???
    "-.-- . .-.. .-.. --- .--", // YELLOW
    "-.-. --- -.. .",           // CODE
    ".-- .... .. - .",          // WHITE
    "-.. -.-. -..- ..",         // DCXI
    "-.. . ...- .. .-..",       // DEVIL
    "-... --- -.-. .-",         // BOCA
    "-.-. .- ... .. -. ---",    // CASINO
    "-... . . .-.",             // BEER
    "--. .-. . . -.",           // GREEN
};

/* ---------- SMOOTHING --------- */

#define SMOOTH_SAMPLES 10

int readings[SMOOTH_SAMPLES];
int readIndex = 0;
int total = 0;

int readSmoothedSensor()
{

    total = total - readings[readIndex];

    readings[readIndex] = analogRead(ROTARY_ANGLE_SENSOR);

    total = total + readings[readIndex];

    readIndex++;

    if (readIndex >= SMOOTH_SAMPLES)
    {
        readIndex = 0;
    }

    return total / SMOOTH_SAMPLES;
}

/* ---------- STATE ---------- */

int currentStation = -1;
bool locked = false;

unsigned long tuneStart = 0;
unsigned long lastMessageTime = 0;

/* ---------- MORSE ---------- */

void beep(int duration)
{
    tone(BUZZER, MORSE_FREQ);
    delay(duration);
    noTone(BUZZER);
}

void playMorse(const char *msg)
{

    for (int i = 0; msg[i] != '\0'; i++)
    {

        char c = msg[i];

        if (c == '.')
        {
            beep(DOT_TIME);
            delay(SYMBOL_SPACE);
        }

        else if (c == '-')
        {
            beep(DASH_TIME);
            delay(SYMBOL_SPACE);
        }

        else if (c == ' ')
        {
            delay(LETTER_SPACE);
        }
    }
}

/* ---------- STATION DETECTION ---------- */

int detectStation(float freq)
{

    for (int i = 0; i < NB_FREQ; i++)
    {
        if (freq > FREQS[i] - MARGIN && freq < FREQS[i] + MARGIN)
        {
            return i;
        }
    }

    return -1;
}

/* ---------- SETUP ---------- */

void setup()
{
    for (int i = 0; i < SMOOTH_SAMPLES; i++)
    {
        readings[i] = analogRead(ROTARY_ANGLE_SENSOR);
        total += readings[i];
    }

    lcd.begin(16, 2);

    pinMode(ROTARY_ANGLE_SENSOR, INPUT);
    pinMode(BUZZER, OUTPUT);

    lcd.setCursor(0, 0);
    lcd.print("Talkie Walkie");
    lcd.setCursor(0, 1);
    lcd.print("Ready");

    delay(2000);
    lcd.clear();
}

/* ---------- LOOP ---------- */

void loop()
{
    /* ----- READ KNOB ----- */

    int sensor_value = readSmoothedSensor();

    float voltage = (float)sensor_value * ADC_REF / 1023;
    float degrees = (voltage * FULL_ANGLE) / GROVE_VCC;

    float freq = MIN_FREQ + (degrees / FULL_ANGLE) * (MAX_FREQ - MIN_FREQ);

    /* ----- DISPLAY ----- */

    lcd.setCursor(0, 0);
    lcd.print("Frequence :");

    lcd.setCursor(0, 1);
    lcd.print(freq, 1);
    lcd.print(" MHz   ");

    /* ----- CHECK STATION ----- */

    int station = detectStation(freq);

    if (station != currentStation)
    {
        currentStation = station;
        tuneStart = millis();
        locked = false;
    }

    if (station == -1)
    {
        delay(40);
        return;
    }

    /* ----- LOCK IF HELD ----- */

    if (!locked && millis() - tuneStart > LOCK_TIME)
    {
        locked = true;
        lastMessageTime = 0;
    }

    /* ----- LOOP MESSAGE ----- */

    if (locked)
    {

        if (millis() - lastMessageTime > MESSAGE_INTERVAL)
        {

            playMorse(MESSAGES[station]);
            lastMessageTime = millis();
        }
    }

    delay(40);
}