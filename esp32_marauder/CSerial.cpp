#include "CSerial.h"
#include <Arduino.h>
#include "configs.h"

CustomSerial CSerial;

void CustomSerial::begin() {
    Serial.begin(115200);

    #ifdef USE_SERIAL1
      Serial1.begin(SERIAL1_BAUD, SERIAL_8N1, SERIAL1_RX, SERIAL1_TX);
    #endif
}

size_t CustomSerial::write(uint8_t c) {
    #ifdef USE_SERIAL1
        Serial1.write(c);
    #endif
    return Serial.write(c);;
}

size_t CustomSerial::write(const uint8_t *buffer, size_t size) {
    #ifdef USE_SERIAL1
        Serial1.write(buffer, size);
    #endif
    return Serial.write(buffer, size);;
}

String CustomSerial::readString() {
    #ifdef USE_SERIAL1
        if (Serial1.available()) {
            return Serial1.readString();
        }
    #endif

    return Serial.readString();
}

String CustomSerial::getSerialInput() {
  String input = "";
  #ifdef USE_SERIAL1
  if (Serial1.available() > 0) {
    input = Serial1.readStringUntil('\n');
  }
  else
  #endif
  if (Serial.available() > 0)
    input = Serial.readStringUntil('\n');

  input.trim();
  return input;
}
