#include "sensor_node.h"

uint8_t readEEPROM_string(uint16_t eeprom_addr, char *data, uint8_t max_len)
{
  uint8_t i, len;
  char c;

  if(!getNodeConfigStatus()) {
    return 0;
  }

  len = EEPROM.read(eeprom_addr);
  eeprom_addr++;
  if(len > max_len || len == 0) {
    return 0;
  }

  for(i = 0; i < len; i++) {
    data[i] = EEPROM.read(eeprom_addr);
    eeprom_addr;
  }

  if(len < max_len) {
    data[len] = '\0';
  }

  return len;
}

uint8_t writeEEPROM_string(uint16_t eeprom_addr, char *data, uint8_t len)
{
  uint8_t i;

  EEPROM.write(eeprom_addr, len);
  eeprom_addr++;
  for(i = 0; i < len; i++) {
    EEPROM.write(eeprom_addr, data[i]);
    eeprom_addr++;
  }

  return len;
}
