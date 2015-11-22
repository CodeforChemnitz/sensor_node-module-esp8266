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


#define NODE_EEPROM_SSID_OFFSET               4
#define NODE_EEPROM_SSID_MAX_LENGTH          64
#define NODE_EEPROM_PASSWORD_OFFSET          69
#define NODE_EEPROM_PASSWORD_MAX_LENGTH      64
#define NODE_EEPROM_API_HOSTNAME_OFFSET     134
#define NODE_EEPROM_API_HOSTNAME_MAX_LENGTH  64
#define NODE_EEPROM_API_PORT_OFFSET         199
#define NODE_EEPROM_API_PORT_SIZE             2
#define NODE_EEPROM_VERSION_MAJOR 0
#define NODE_EEPROM_VERSION_MINOR 0

#define NODE_API_DEFAULT_HOSTNAME ""

#define SENSOR_NODE_UUID_MAX_LENGTH 64
#define SENSOR_NODE_KEY_MAX_LENGTH  64

template <uint16_t SIZE>
class DataString : public Print
{
  public:
    DataString() { this->length = 0; };
    void reset() { this->length = 0; };
    virtual size_t write(uint8_t c) {
      this->data[this->length] = c;
      this->length++;
    };
    uint16_t length;
    uint8_t data[SIZE];
  private:
    uint16_t max_length = SIZE;
};

class ArduRPC_SensorNode : public ArduRPCHandler
{
  public:
    ArduRPC_SensorNode(ArduRPC &rpc, char *name);
    uint8_t
      ICACHE_FLASH_ATTR call(uint8_t);
    void ICACHE_FLASH_ATTR submitData();
  private:
    uint8_t status;
    DataString<1024> cache;
    char sensor_uuid[SENSOR_NODE_UUID_MAX_LENGTH + 1];
    char sensor_key[SENSOR_NODE_UUID_MAX_LENGTH + 1];
};

void ICACHE_FLASH_ATTR handleAPIHostname();
void ICACHE_FLASH_ATTR handleAPIPort();
void ICACHE_FLASH_ATTR handleNotFound();
void ICACHE_FLASH_ATTR handlePassword();
void ICACHE_FLASH_ATTR handleRegister();
void ICACHE_FLASH_ATTR handleRestart();
void ICACHE_FLASH_ATTR handleRoot();
void ICACHE_FLASH_ATTR handleSave();
void ICACHE_FLASH_ATTR handleScanSSID();
void ICACHE_FLASH_ATTR handleSSID();
void ICACHE_FLASH_ATTR submitFile(PGM_VOID_P, uint16_t);

extern ESP8266WebServer *server;
extern WiFiClient *client;

// EEPROM helper functions
uint8_t ICACHE_FLASH_ATTR readEEPROM_string(uint16_t, char *, uint8_t);
uint8_t ICACHE_FLASH_ATTR writeEEPROM_string(uint16_t, char *, uint8_t);

// Config
uint8_t ICACHE_FLASH_ATTR getAPIHostnameOrDefault(char *, uint8_t);
uint16_t ICACHE_FLASH_ATTR getAPIPortOrDefault();

bool ICACHE_FLASH_ATTR waitWiFiClientConnected(uint8_t);
bool ICACHE_FLASH_ATTR connectWiFiClient(uint8_t connect_timeout);
uint8_t ICACHE_FLASH_ATTR setAPIHostname(char *, uint8_t);
bool ICACHE_FLASH_ATTR setAPIPort(uint16_t);
uint8_t ICACHE_FLASH_ATTR setWiFiSSID(char *ssid, uint8_t len);
uint8_t ICACHE_FLASH_ATTR setWiFiPassword(char *password, uint8_t len);
uint8_t ICACHE_FLASH_ATTR getAPIHostname(char *, uint8_t);
uint16_t ICACHE_FLASH_ATTR getAPIPort();
uint8_t ICACHE_FLASH_ATTR getWiFiPassword(char *password, uint8_t max_len);
uint8_t ICACHE_FLASH_ATTR getNodeConfigStatus();
uint8_t ICACHE_FLASH_ATTR getWiFiSSID(char *ssid, uint8_t max_len);
void ICACHE_FLASH_ATTR initConfig();

WiFiClient ICACHE_FLASH_ATTR *connectSensorAPI();

#endif
