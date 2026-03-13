#include "master.h"

void readButtons()
{
    unsigned long now = millis();
    for (uint8_t i = 0; i < NUM_BTN; i++)
    {
        bool r = digitalRead(pins[i]);
        if (r != rawState[i])
        {
            rawState[i] = r;
            debounceTime[i] = now;
        }
        if ((now - debounceTime[i]) > DEBOUNCE && buttonState[i] != rawState[i])
        {
            buttonState[i] = rawState[i];
            digitalWrite(leds[i], !buttonState[i]);
            if (buttonState[i] == LOW)
            {
                pressTime[i] = now;
                longTriggered[i] = false;
            }
            else
            {
                if (holdButton == i)
                    releaseHold(i);
                else if (!longTriggered[i])
                    shortPress(i);
            }
        }
        // Appui long
        if (buttonState[i] == LOW && !longTriggered[i] && (now - pressTime[i]) >= LONG_PRESS)
        {
            longTriggered[i] = true;
            longPress(i);
        }
    }
}

bool anyHoldActive()
{
    for (uint8_t i = 0; i < NUM_BTN; i++)
        if (buttonState[i] == LOW)
            return true;
    return false;
}

void addEvent(SequenceType t, uint8_t pin)
{
    if (inputCount < INPUT_MAX)
    {
        inputSeq[inputCount++] = {t, pin};
        inputWindowStart = millis();
    }
}

void shortPress(uint8_t i)
{
    Serial.print("[BTN] short: ");
    Serial.println(names[i]);
    addEvent(SHORT_INPUT, pins[i]);
}
void longPress(uint8_t i)
{
    Serial.print("[BTN] long:  ");
    Serial.println(names[i]);
    holdButton = i;
    addEvent(PRESS_INPUT, pins[i]);
}
void releaseHold(uint8_t i)
{
    Serial.print("[BTN] release: ");
    Serial.println(names[i]);
    addEvent(RELEASE_INPUT, pins[i]);
    holdButton = -1;
}

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
bool checkSequence(uint8_t idx)
{
    if (inputCount != seqLen[idx])
        return false;
    for (uint8_t i = 0; i < inputCount; i++)
    {
        if (inputSeq[i].type != sequences[idx][i].type)
            return false;
        if (inputSeq[i].pin != sequences[idx][i].pin)
            return false;
    }
    return true;
}

char (*seqPrint(SequenceEvent *seq))[17]
{
    static char lines[2][17];
    lines[0][0] = '\0';
    lines[1][0] = '\0';

    char line1[17] = "";
    char line2[17] = "";

    uint8_t maxEvents = (inputCount > 10) ? 10 : inputCount;

    for (uint8_t i = 0; i < maxEvents; i++)
    {
        const char *label = "?";
        for (uint8_t j = 0; j < NUM_BTN; j++)
        {
            if (pins[j] == seq[i].pin)
            {
                label = names[j];
                break;
            }
        }
        char token[8];
        if (seq[i].type == SHORT_INPUT)
        {
            snprintf(token, sizeof(token), "%s ", label);
        }
        else if (seq[i].type == PRESS_INPUT)
        {
            snprintf(token, sizeof(token), "%s:", label);
        }
        else
        {
            snprintf(token, sizeof(token), ":%s", label);
        }

        if (i < 5)
        {
            strncat(line1, token, sizeof(line1) - strlen(line1) - 1);
        }
        else
        {
            strncat(line2, token, sizeof(line2) - strlen(line2) - 1);
        }
    }

    size_t l1 = strlen(line1);
    if (l1 >= 16)
        line1[16] = '\0';
    else
        for (size_t k = l1; k < 16; k++)
            line1[k] = ' ';
    line1[16] = '\0';

    size_t l2 = strlen(line2);
    if (l2 >= 16)
        line2[16] = '\0';
    else
        for (size_t k = l2; k < 16; k++)
            line2[k] = ' ';
    line2[16] = '\0';

    memcpy(lines[0], line1, 17);
    memcpy(lines[1], line2, 17);
    return lines;
}

/* ----------------- LCD DISPLAY ----------------- */
void lcdShow(uint8_t line, const char *text)
{
    lcd.setCursor(0, line - 1);
    lcd.print("                ");
    lcd.setCursor(0, line - 1);
    lcd.print(text);
}

/* ---------------- LORA SEND (non-bloquant) ---------------- */
void loraSend(uint8_t *payload, uint8_t size)
{
    LoRa_Serial.print("AT+MSGHEX=\"");
    for (uint8_t i = 0; i < size; i++)
    {
        if (payload[i] < 0x10)
            LoRa_Serial.print("0");
        LoRa_Serial.print(payload[i], HEX);
    }
    LoRa_Serial.println("\"");
}

/* ---------------- TRANSMISSION ---------------- */
void sendCmd(const char *key, const char *val)
{
    Serial.print("[TX] ");
    Serial.print(key);
    if (val)
    {
        Serial.print(":");
        Serial.println(val);
    }
    else
        Serial.println();
    slaveSerial.print(key);
    if (val)
    {
        slaveSerial.print(":");
        slaveSerial.println(val);
    }
    else
        slaveSerial.println();
}

void sendCmd(const char *key, int val)
{
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", val);
    sendCmd(key, buf);
}

/* ---------------- GESTION CARTE RFID ---------------- */
void handleCard()
{
    bool ok = (rfid.uid.size == uidWinSize);
    for (byte i = 0; i < uidWinSize && ok; i++)
        if (rfid.uid.uidByte[i] != uidWin[i])
            ok = false;

    Serial.print("[RFID] carte ");
    Serial.println(ok ? "OK" : "NOK");

    if (ok)
    {
        lcd.setRGB(0, 255, 0);
        lcdShow(1, "BRAVO !");
        lcdShow(2, "Acces autorise");
        sendCmd("leds", 4);
        sendCmd("buzz", 7);
        sendCmd("col", 2);
        sendCmd("open");
        gameState = WIN;
    }
    else
    {
        lcd.setRGB(255, 0, 0);
        lcdShow(1, "Mauvaise carte!");
        lcdShow(2, "Reessayez...");
        sendCmd("buzz", 3);
        loraSend(payloadUpNOK, sizeof(payloadUpNOK));
    }
}

/* ------------------- SETUP ------------------- */
void setup()
{
    Serial.begin(DEBUG_BAUD);
    slaveSerial.begin(SLAVE_BAUD);
    lcd.begin(LCD_COLS, LCD_ROWS);
    lcdShow(1, "Systeme");
    Serial.println("[MASTER] boot");
    for (uint8_t i = 0; i < NUM_BTN; i++)
    {
        pinMode(pins[i], INPUT_PULLUP);
        pinMode(leds[i], OUTPUT);
    }

    // RFID init
    SPI.begin();
    rfid.PCD_Init();
    rfid.PCD_SetAntennaGain(rfid.RxGain_max);
    Serial.println("[RFID] ready");

    // LoRaWAN init
    lorae5.setup_hardware(&Debug_Serial, &LoRa_Serial);
    lorae5.setup_lorawan(REGION, ACTIVATION_MODE, CLASS, SPREADING_FACTOR, ADAPTIVE_DR, CONFIRMED, PORT_UP, false, JOIN_TIMEOUT_MS);
    if (ACTIVATION_MODE == OTAA)
    {
        lcdShow(2, "Loading...");
        unsigned long joinStart = millis();
        bool joined = false;
        while (!joined && (millis() - joinStart) < JOIN_TIMEOUT_MS)
        {
            joined = lorae5.join();
        }
        if (!joined)
        {
            Serial.println("[LORA] join timeout");
            lcdShow(2, "LoRa fail");
        }
        delay(2000);
    }

    sendCmd("leds", 1);
    lcdShow(1, "Entrez la");
    {
        char buf[17];
        snprintf(buf, sizeof(buf), "sequence 1/%d", NB_SEQ);
        lcdShow(2, buf);
    }
    inputWindowStart = millis();
}

/* ------------------- LOOP ------------------- */
void loop()
{

    if (gameState == WIN)
        return;

    /* -------- SEQUENCES -------- */
    if (gameState == WAIT_SEQ)
    {
        readButtons();

        if (inputCount > 0 && !anyHoldActive() && millis() - inputWindowStart > SEQ_TIMEOUT_MS)
        {
            char (*lines)[17] = seqPrint(inputSeq);
            sendMsg1(lines[0]);
            sendMsg2(lines[1]);
            if (checkSequence(currentSeq))
            {
                currentSeq++;
                lcd.setRGB(0, 255, 0);
                lcdShow(2, "Code valide !");
                sendCmd("buzz", 4);

                if (currentSeq == NB_SEQ)
                {
                    lcdShow(1, "Passez la carte");
                    lcdShow(2, "");
                    gameState = WAIT_CARD;
                    Serial.println("[GAME] toutes les sequences OK -> WAIT_CARD");
                }
                else
                {
                    sendCmd("leds", currentSeq + 1);
                    lcdShow(1, "Entrez la");
                    char buf[17];
                    snprintf(buf, sizeof(buf), "sequence %d/%d", currentSeq + 1, NB_SEQ);
                    lcdShow(2, buf);
                }
            }
            else
            {
                lcd.setRGB(255, 0, 0);
                lcdShow(1, "Code invalide!");
                lcdShow(2, "Recommencez...");
                sendCmd("buzz", 3);
                seqIndex = 0;
                errorTimer = millis();
                gameState = SEQ_ERROR;
                Serial.println("[GAME] sequence NOK -> SEQ_ERROR");
            }

            inputCount = 0;
            inputWindowStart = millis();
        }
        else if (inputCount == 0)
        {
            inputWindowStart = millis();
        }
    }

    /* -------- ERREUR SÉQUENCE -------- */
    if (gameState == SEQ_ERROR)
    {
        if (millis() - errorDisplayStart >= ERROR_DISPLAY_MS)
        {
            lcd.setRGB(255, 255, 255);
            sendCmd("leds", 1);
            lcdShow(1, "Entrez la");
            {
                char buf[17];
                snprintf(buf, sizeof(buf), "sequence 1/%d", NB_SEQ);
                lcdShow(2, buf);
            }
            gameState = WAIT_SEQ;
            Serial.println("[GAME] reset -> WAIT_SEQ");
        }
    }

    /* -------- PHASE CARTE -------- */
    if (gameState == WAIT_CARD)
    {
        if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
        {
            handleCard();
            rfid.PICC_HaltA();
            rfid.PCD_StopCrypto1();
        }
    }
}
