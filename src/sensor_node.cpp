#include "sensor_node.h"
#include <ESP8266WiFi.h>

SensorNodeESP8266::SensorNodeESP8266() : ArduRPCHandler()
{
  this->type = 0xff10;
}

SensorNodeESP8266::SensorNodeESP8266(ArduRPC &rpc, char *name) : ArduRPCHandler()
{
  this->type = 0xff10;
  //this->registerSelf(rpc, name);
}

ICACHE_FLASH_ATTR uint8_t SensorNodeESP8266::call(uint8_t cmd_id)
{
  uint8_t u8_tmp1;
  uint16_t u16_tmp1;
  uint32_t u32_tmp1;

  if(cmd_id == 0x11) {
  } else {
    return RPC_RETURN_COMMAND_NOT_FOUND;
  }
  return RPC_RETURN_SUCCESS;
}
