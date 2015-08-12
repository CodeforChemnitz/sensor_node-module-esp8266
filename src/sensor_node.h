#ifndef ARDURPC_ESP8266_H
#define ARDURPC_ESP8266_H

#include <ESP8266WiFi.h>
#include "ArduRPC.h"

#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #error "Use Arduino IDE >= 1.0"
#endif

class SensorNodeESP8266 : public ArduRPCHandler
{
  public:
    SensorNodeESP8266();
    SensorNodeESP8266(ArduRPC &, char *);
    uint8_t
      call(uint8_t);

};

#endif
