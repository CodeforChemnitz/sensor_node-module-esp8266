#ifndef ARDURPC_ESP8266_H
#define ARDURPC_ESP8266_H

#include <ESP8266WiFi.h>
#include "ArduRPC.h"

#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #error "Use Arduino IDE >= 1.0"
#endif

class DataString : public Print
{
  public:
    DataString(uint16_t);
    void reset();
    virtual size_t write(uint8_t);
    uint16_t length;
    uint8_t *data;
  private:
    uint16_t max_length;
};

class ArduRPC_SensorNode : public ArduRPCHandler
{
  public:
    ArduRPC_SensorNode(ArduRPC &rpc, char *name);
    uint8_t
      call(uint8_t);
  private:
    uint8_t status;
    DataString *cache;
};

#endif
