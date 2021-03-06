#include "sensor_node.h"
#include <ESP8266HTTPClient.h>

ArduRPC_SensorNode::ArduRPC_SensorNode(ArduRPC &rpc, char *name) : ArduRPCHandler()
{
  this->type = 0x9999;
  this->registerSelf(rpc, name, (void *)this);
  this->status = 0;
}

uint8_t ArduRPC_SensorNode::call(uint8_t cmd_id)
{
  uint16_t i;

  if (cmd_id == 0x10) {
    /* start() */
    if(WiFi.status() != WL_CONNECTED) {
      connectWiFiClient(0);
    }

    // get Params UUID and API KEY
    this->_rpc->getParam_string(&this->sensor_uuid[0], SENSOR_NODE_UUID_MAX_LENGTH);
    this->sensor_uuid[SENSOR_NODE_UUID_MAX_LENGTH] = '\0';

    this->_rpc->getParam_string(&this->sensor_key[0], SENSOR_NODE_KEY_MAX_LENGTH);
    this->sensor_key[SENSOR_NODE_KEY_MAX_LENGTH] = '\0';

    this->status = 2;
    this->cache.reset();
    this->cache.print("[");
    return RPC_RETURN_SUCCESS;
  } else if (cmd_id == 0x11) {
    /* finish() */
    if(this->status == 2 or this->status == 3) {
      this->cache.print("]");
    }
    this->status = 4;
    this->submitData();

    return RPC_RETURN_SUCCESS;
  } else if (cmd_id == 0x12) {
    /* getStatus() */
    if(this->status == 4) {
      this->submitData();
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
      this->cache.print(",");
    }

    this->cache.print("[");
    // sensor id
    u8 = this->_rpc->getParam_uint8();
    this->cache.print(u8);
    this->cache.print(",");
    // sensor type
    u16 = this->_rpc->getParam_uint16();
    this->cache.print(u16);
    this->cache.print(",");
    // value type
    u8 = this->_rpc->getParam_uint8();
    this->cache.print(u8);
    this->cache.print(",");
    // value
    // - read value type
    u8 = this->_rpc->getParam_uint8();
    // - read value
    if(u8 == RPC_INT8) {
      i8 = this->_rpc->getParam_int8();
      this->cache.print(i8);
    } else if(u8 == RPC_UINT8) {
      u8 = this->_rpc->getParam_uint8();
      this->cache.print(u8);
    } else if(u8 == RPC_INT16) {
      i16 = this->_rpc->getParam_int16();
      this->cache.print(i16);
    } else if(u8 == RPC_UINT16) {
      u16 = this->_rpc->getParam_uint16();
      this->cache.print(u16);
    } else if(u8 == RPC_INT32) {
      i32 = this->_rpc->getParam_int32();
      this->cache.print(i32);
    } else if(u8 == RPC_UINT32) {
      u32 = this->_rpc->getParam_uint32();
      this->cache.print(u32);
    } else if(u8 == RPC_FLOAT) {
      f = this->_rpc->getParam_float();
      this->cache.print(f);
    } else {
      // value type not supported
      this->cache.print("\"n/a\"");
    }
    this->cache.print("]");
    return RPC_RETURN_SUCCESS;
  }
  return RPC_RETURN_COMMAND_NOT_FOUND;
}

void ArduRPC_SensorNode::submitData()
{
  uint16_t i;
  char hostname[NODE_EEPROM_API_HOSTNAME_MAX_LENGTH + 1];
  uint16_t port;

  if(this->status != 4) {
    return;
  }

  if(WiFi.status() != WL_CONNECTED) {
    return;
  }

/*
  if(connectSensorAPI() == false) {
    return;
  }
*/
  getAPIHostnameOrDefault(&hostname[0], NODE_EEPROM_API_HOSTNAME_MAX_LENGTH);
  port = getAPIPortOrDefault();

  HTTPClient http;

  String path;
  path.reserve(64);
  path = "/sensors/";
  path += this->sensor_uuid;

  http.begin(hostname, port, path);
  http.addHeader("X-Sensor-Api-Key", this->sensor_key);
  http.addHeader("X-Sensor-Version", "1");
  int code;
  code = http.POST(this->cache.data, this->cache.length);
  NODE_DEBUG_PRINT("Code ");
  NODE_DEBUG_PRINTLN(code);

  // ToDo: validate code
  this->status = 0;

/*
  client.print("POST /sensors/");
  client.println(this->sensor_uuid);
  client.print("Host: ");
  client.println(hostname);
  client.println("Connection: close");
  client.print("X-Sensor-Api-Key: ");
  client.println(this->sensor_key);
  client.println("X-Sensor-Version: 1");
  client.print("Content-Length: ");
  client.println(this->cache.length);
  client.println();
  for(i = 0; i < this->cache.length; i++) {
    Serial.print((char)this->cache.data[i]);
  }
  this->status = 0;
  client.stop();
*/
}
