#include "sensor_node.h"

uint8_t getNodeConfigStatus()
{
  uint8_t status;
  uint8_t version_major, version_minor;

  status = EEPROM.read(0);
  version_major = EEPROM.read(1);
  version_minor = EEPROM.read(2);

  if(status != 0xAA && version_major == NODE_EEPROM_VERSION_MAJOR) {
    return 0;
  }
  return 1;
}

uint8_t getAPIHostname(char *hostname, uint8_t max_len)
{
  return readEEPROM_string(NODE_EEPROM_API_HOSTNAME_OFFSET, hostname, max_len);
}

uint8_t getAPIHostnameOrDefault(char *hostname, uint8_t max_len)
{
  uint8_t len;
  char default_hostname[] = NODE_API_DEFAULT_HOSTNAME;
  len = getAPIHostname(hostname, max_len);
  if(len > 0) {
    return len;
  }
  strncpy(hostname, default_hostname, max_len);
  return strlen(hostname);
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

  // initialize eeprom with empty values
  char empty[0];
  setAPIHostname(&empty[0], 0);
  setAPIPort(0);
  setWiFiSSID(&empty[0], 0);
  setWiFiPassword(&empty[0], 0);

  // Set config version
  EEPROM.write(0, 0xAA);
  EEPROM.write(1, NODE_EEPROM_VERSION_MAJOR);
  EEPROM.write(2, NODE_EEPROM_VERSION_MINOR);
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
