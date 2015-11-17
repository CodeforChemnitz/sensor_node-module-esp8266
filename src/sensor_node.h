#ifndef ARDURPC_ESP8266_H
#define ARDURPC_ESP8266_H

#include <ESP8266WiFi.h>
#include "ArduRPC.h"

#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #error "Use Arduino IDE >= 1.0"
#endif
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <EEPROM.h>

#define NODE_MODE_ACTIVE 0
#define NODE_MODE_CONFIG 1

#define NODE_EEPROM_SSID_OFFSET 1
#define NODE_EEPROM_PASSWORD_OFFSET 66


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

void handleNotFound();
void handlePassword();
void handleRegister();
void handleRestart();
void handleRoot();
void handleSave();
void handleScanSSID();
void handleSSID();

extern ESP8266WebServer *server;
extern WiFiClient *client;

bool waitWiFiClientConnected(uint8_t);
bool waitWiFiClientConnected(uint8_t connect_timeout);
bool connectWiFiClient(uint8_t connect_timeout);
uint8_t setWiFiSSID(char *ssid, uint8_t len);
uint8_t setWiFiPassword(char *password, uint8_t len);
uint8_t getWiFiPassword(char *password, uint8_t max_len);
uint8_t getNodeConfigStatus();
uint8_t getWiFiSSID(char *ssid, uint8_t max_len);
void initConfig();

#endif
