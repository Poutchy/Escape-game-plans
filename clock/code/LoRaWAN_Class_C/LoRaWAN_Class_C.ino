#include <Arduino.h>
#include "lorae5.h"
#include "config_application.h"
#include "config_board.h"

const int READYSWITCH = 2;
const int STOPBUTTON = 4;
const int PENALITYBUTTON = 7;
int button = 4;

uint8_t sizePayloadUp = 4;
uint8_t sizePayloadDown = 0;

uint8_t payloadUp[20] = {0x00, 0x01, 0x02, 0x03};
uint8_t payloadDown[20]  ={0};

LORAE5 lorae5(devEUI, appEUI, appKey, devAddr, nwkSKey, appSKey);

/***********************************************************************/
/* Please see README page on https://github.com/SylvainMontagny/LoRaE5 */
/***********************************************************************/

void setup() {
  pinMode(READYSWITCH, OUTPUT);
  pinMode(STOPBUTTON, OUTPUT);
  pinMode(PENALITYBUTTON, OUTPUT);
  digitalWrite(READYSWITCH, LOW);
  digitalWrite(STOPBUTTON, LOW);
  digitalWrite(PENALITYBUTTON, LOW);

  lorae5.setup_hardware(&Debug_Serial, &LoRa_Serial);
  lorae5.setup_lorawan(REGION, ACTIVATION_MODE, CLASS, SPREADING_FACTOR, ADAPTIVE_DR, CONFIRMED, PORT_UP, SEND_BY_PUSH_BUTTON, FRAME_DELAY);
  lorae5.printInfo();

  if(ACTIVATION_MODE == OTAA){
    Debug_Serial.println("Join Procedure in progress...");
    while(lorae5.join() == false);
    delay(2000);
  }

  digitalWrite(READYSWITCH, HIGH);
  delay(5);
  digitalWrite(READYSWITCH, LOW);
}

void loop() {
  // Send data
  lorae5.sendData(payloadUp, sizePayloadUp);

  // Check downlink on RXC and call processDownlink() 
  if (lorae5.awaitForDownlinkClass_C(payloadDown, &sizePayloadDown) == RET_DOWNLINK){
    processDownlink();
  };
}

void processDownlink() {
  if (payloadDown[0] == 0)
    button = STOPBUTTON;
  else
    button = PENALITYBUTTON;

  digitalWrite(button, HIGH);
  delay(10);
  digitalWrite(button, LOW);
} 