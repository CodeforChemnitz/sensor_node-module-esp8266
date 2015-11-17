#include "sensor_node.h"

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
    int8_t i8;
    uint8_t u8;
    int16_t i16;
    uint16_t u16;
    int32_t i32;
    uint32_t u32;
    float f;

    if(this->status == 2) {
      this->status = 3;
    } else {
      this->cache->print(",");
    }

    this->cache->print("[");
    // sensor id
    u8 = this->_rpc->getParam_uint8();
    this->cache->print(u8);
    this->cache->print(",");
    // sensor type
    u16 = this->_rpc->getParam_uint16();
    this->cache->print(u16);
    this->cache->print(",");
    // value type
    u8 = this->_rpc->getParam_uint8();
    this->cache->print(u8);
    this->cache->print(",");
    // value
    // - read value type
    u8 = this->_rpc->getParam_uint8();
    // - read value
    if(u8 == RPC_INT8) {
      i8 = this->_rpc->getParam_int8();
      this->cache->print(i8);
    } else if(u8 == RPC_UINT8) {
      u8 = this->_rpc->getParam_uint8();
      this->cache->print(u8);
    } else if(u8 == RPC_INT16) {
      i16 = this->_rpc->getParam_int16();
      this->cache->print(i16);
    } else if(u8 == RPC_UINT16) {
      u16 = this->_rpc->getParam_uint16();
      this->cache->print(u16);
    } else if(u8 == RPC_INT32) {
      i32 = this->_rpc->getParam_int32();
      this->cache->print(i32);
    } else if(u8 == RPC_UINT32) {
      u32 = this->_rpc->getParam_uint32();
      this->cache->print(u32);
    } else if(u8 == RPC_FLOAT) {
      f = this->_rpc->getParam_float();
      this->cache->print(f);
    } else {
      // value type not supported
      this->cache->print("\"n/a\"");
    }
    this->cache->print("]");
    return RPC_RETURN_SUCCESS;
  }
  return RPC_RETURN_COMMAND_NOT_FOUND;
}
