#include "sensor_node.h"
#include <ESP8266WiFi.h>


ArduRPC_SensorNode::ArduRPC_SensorNode(ArduRPC &rpc, char *name) : ArduRPCHandler()
{
  this->type = 0x9999;
  this->registerSelf(rpc, name, (void *)this);
}

uint8_t ArduRPC_SensorNode::call(uint8_t cmd_id)
{
  if (cmd_id == 0x10) {
    /* start() */
    return RPC_RETURN_SUCCESS;
  } else if (cmd_id == 0x11) {
    /* finish() */
  } else if (cmd_id == 0x12) {
    /* getStatus() */
  } else if (cmd_id == 0x13) {
    /* submitValue() */
  }
  // this->_rpc->writeResult_uint8(3);
  return RPC_RETURN_COMMAND_NOT_FOUND;
}
