#include <Arduino.h>
#include "lorae5.h"
#include "config_application.h"
#include "config_board.h"

uint8_t sizePayloadUp = 1;
uint8_t sizePayloadDown = 0;

uint8_t payloadUpNOK[5] = {0x01};
uint8_t payloadUpALIVE[5] = {0x02};
uint8_t payloadDown[20] = {0};

LORAE5 lorae5(devEUI, appEUI, appKey, devAddr, nwkSKey, appSKey);

/***********************************************************************/
/* Please see README page on https://github.com/SylvainMontagny/LoRaE5 */
/***********************************************************************/

void setup() {
  lorae5.setup_hardware(&Debug_Serial, &LoRa_Serial);
  lorae5.setup_lorawan(REGION, ACTIVATION_MODE, CLASS, SPREADING_FACTOR, ADAPTIVE_DR, CONFIRMED, PORT_UP, SEND_BY_PUSH_BUTTON, FRAME_DELAY);
  lorae5.printInfo();

  if(ACTIVATION_MODE == OTAA){
    Debug_Serial.println("Join Procedure in progress...");
    while(lorae5.join() == false);
    delay(2000);
   }
}

void loop() {
  bool failed = true;
  if (failed) {
    send(payloadUp);
    failed = false;
  }
}

void send(uint8_t payloadUp) {
  lorae5.sendData(uint8_t *payloadUp, uint8_t sizePayloadUp);
}

