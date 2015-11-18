#include "sensor_node.h"

uint8_t getNodeConfigStatus()
{
  uint8_t status;
  status = EEPROM.read(0);

  if(status != 0x33) {
    return 0;
  }
  return 1;
}

uint8_t getAPIHostname(char *hostname, uint8_t max_len)
{
  return readEEPROM_string(NODE_EEPROM_API_HOSTNAME_OFFSET, hostname, max_len);
}

uint16_t getAPIPort()
{
  uint16_t port;
  EEPROM.get(NODE_EEPROM_API_PORT_OFFSET, port);
  return port;
}

uint8_t getWiFiPassword(char *password, uint8_t max_len)
{
  return readEEPROM_string(NODE_EEPROM_PASSWORD_OFFSET, password, max_len);
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

uint8_t setAPIHostname(char *hostname, uint8_t len)
{
  return writeEEPROM_string(NODE_EEPROM_API_HOSTNAME_OFFSET, hostname, len);
}

bool setAPIPort(uint16_t port)
{
  EEPROM.put(NODE_EEPROM_API_PORT_OFFSET, port);
  return true;
}

uint8_t setWiFiPassword(char *password, uint8_t len)
{
  return writeEEPROM_string(NODE_EEPROM_PASSWORD_OFFSET, password, len);
}

uint8_t setWiFiSSID(char *ssid, uint8_t len)
{
  return writeEEPROM_string(NODE_EEPROM_SSID_OFFSET, ssid, len);
}
