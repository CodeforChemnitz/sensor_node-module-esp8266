#include "sensor_node.h"

bool connectWiFiClient(uint8_t connect_timeout=0)
{
  char ssid[65];
  char password[65];

  uint8_t len;

  len = getWiFiSSID(&ssid[0], 64);
  ssid[64] = '\0';
  Serial.println(ssid);
  if(len == 0) {
    Serial.println("exit");
    return false;
  }

  len = getWiFiPassword(&password[0], 64);
  password[64] = '\0';
  Serial.println(password);
  Serial.println(len);
  if(len == 0) {
    WiFi.begin(ssid);
  } else {
    WiFi.begin(ssid, password);
  }

  if(connect_timeout == 0) {
    return true;
  } else {
    return waitWiFiClientConnected(connect_timeout);
  }
}

bool waitWiFiClientConnected(uint8_t connect_timeout)
{
  uint8_t i;

  for(i = 0; i < connect_timeout * 2; i++) {  
    if(WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected");
      return true;
    }
    Serial.println("Waiting ...");
    delay(500);
  }
  return false;
}

uint8_t setWiFiSSID(char *ssid, uint8_t len)
{
  return writeEEPROM_string(NODE_EEPROM_SSID_OFFSET, ssid, len);
}

uint8_t setWiFiPassword(char *password, uint8_t len)
{
  return writeEEPROM_string(NODE_EEPROM_PASSWORD_OFFSET, password, len);
}

uint8_t getWiFiPassword(char *password, uint8_t max_len)
{
  return readEEPROM_string(NODE_EEPROM_PASSWORD_OFFSET, password, max_len);
}

uint8_t getNodeConfigStatus()
{
  uint8_t status;
  status = EEPROM.read(0);

  if(status != 0x33) {
    return 0;
  }
  return 1;
}

uint8_t getWiFiSSID(char *ssid, uint8_t max_len)
{
  return readEEPROM_string(NODE_EEPROM_SSID_OFFSET, ssid, max_len);
}

void initConfig()
{
  if(getNodeConfigStatus()) {
    return;
  }
  EEPROM.write(NODE_EEPROM_SSID_OFFSET, 0x00);
  EEPROM.write(NODE_EEPROM_PASSWORD_OFFSET, 0x00);
  EEPROM.write(0, 0x33);
  EEPROM.commit();
}
