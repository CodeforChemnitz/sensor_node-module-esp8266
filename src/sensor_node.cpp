#include "sensor_node.h"
#include <ESP8266WiFi.h>


DataString::DataString(uint16_t max_length)
{
  this->max_length = max_length;
  this->data = (uint8_t *)malloc(sizeof(uint8_t) * max_length);
  this->length = 0;
}

void DataString::reset()
{
  this->length = 0;
}

size_t DataString::write(uint8_t c)
{
  this->data[this->length] = c;
  this->length++;
}

ArduRPC_SensorNode::ArduRPC_SensorNode(ArduRPC &rpc, char *name) : ArduRPCHandler()
{
  this->type = 0x9999;
  this->registerSelf(rpc, name, (void *)this);
  this->status = 0;
  this->cache = new DataString(1024);
}

uint8_t ArduRPC_SensorNode::call(uint8_t cmd_id)
{
  uint16_t i;

  if (cmd_id == 0x10) {
    /* start() */
    // ToDo: use connect wifi function and status = 1
    //connectWiFiClient(1);
    this->status = 2;
    this->cache->reset();
    this->cache->print("[");
    return RPC_RETURN_SUCCESS;
  } else if (cmd_id == 0x11) {
    this->cache->print("]");
    /* finish() */
    // ToDo: change to client, only for debugging
    //if(client.connect("", 80)) {
      Serial.println("POST /sensor/data");
      Serial.println("Host: host");
      Serial.println("Connection: close");
      Serial.print("Content-Length: ");
      Serial.println(this->cache->length);
      Serial.println();
      for(i = 0; i < this->cache->length; i++) {
        Serial.print((char)this->cache->data[i]);
      }
    //}
    return RPC_RETURN_SUCCESS;
  } else if (cmd_id == 0x12) {
    /* getStatus() */
    if(this->status == 1) {
      if(WiFi.status() == WL_CONNECTED) {
        this->status = 2;
      }
    }
    this->_rpc->writeResult_uint8(this->status);
    return RPC_RETURN_SUCCESS;
  } else if (cmd_id == 0x13) {
    /* submitValue() */
    uint8_t u8;
    uint16_t u16;
    Serial.println("value");
    this->cache->print("[");
    u8 = this->_rpc->getParam_uint8();
    Serial.println(u8);
    this->cache->print(u8);
    this->cache->print(",");
    u16 = this->_rpc->getParam_uint16();
    Serial.println(u16);
    this->cache->print(u16);
    this->cache->print(",");
    this->cache->print("]");
    return RPC_RETURN_SUCCESS;
  }
  // this->_rpc->writeResult_uint8(3);
  return RPC_RETURN_COMMAND_NOT_FOUND;
}
