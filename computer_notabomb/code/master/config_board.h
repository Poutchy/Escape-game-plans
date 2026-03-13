#ifndef CONFIG_BOARD_H
#define CONFIG_BOARD_H

//////////////////////////////////////////////
// Arduino UNO R4 (Renesas RA4M1)
//////////////////////////////////////////////
  #if defined(ARDUINO_ARCH_RENESAS_UNO) || defined(ARDUINO_ARCH_RENESAS)
    #define Debug_Serial      Serial   // USB serial for debug
    #define LoRa_Serial       Serial1  // Serial1 for LoRa-E5 (slave uses SoftwareSerial on A0/A1)

//////////////////////////////////////////////
// Arduino AVR / SAM / SAMD / ESP32
//////////////////////////////////////////////
  #elif defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_WIO_TERMINAL)
    #define Debug_Serial      Serial
    #define LoRa_Serial       Serial1

//////////////////////////////////////////////
// STM32 NUCLEO
//////////////////////////////////////////////
  #elif defined(ARDUINO_ARCH_STM32)
    #define RX_PIN_DEBUG      PA3
    #define TX_PIN_DEBUG      PA2
    HardwareSerial Debug_Serial(RX_PIN_DEBUG, TX_PIN_DEBUG);

    #define RX_PIN_LORA       PA1
    #define TX_PIN_LORA       PA0
    HardwareSerial LoRa_Serial(RX_PIN_LORA, TX_PIN_LORA);

  #else
    #warning "Board not tested — defaulting Serial/Serial1"
    #define Debug_Serial      Serial
    #define LoRa_Serial       Serial1
  #endif

#endif // CONFIG_BOARD_H
