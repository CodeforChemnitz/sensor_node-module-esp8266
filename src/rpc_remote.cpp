#include "sensor_node.h"

ArduRPC_SensorNodeRemote::ArduRPC_SensorNodeRemote(ArduRPCRequest &rpc, uint8_t handler_id) : ArduRPCRequestHandler()
{
  this->_rpc = &rpc;
  this->_handler_id = handler_id;
}

uint8_t ArduRPC_SensorNodeRemote::getMaxSensorCount()
{
  uint8_t value;

  this->_rpc->reset();
  this->_rpc->call(this->_handler_id, 0x09);
  
  value = this->_rpc->readResult_uint8();
  return value;
}

uint8_t ArduRPC_SensorNodeRemote::getSensorConfig(uint8_t sensor_id, uint8_t *data, uint8_t max_length)
{
  uint8_t type;
  uint8_t length;
  uint8_t i;

  this->_rpc->reset();
  this->_rpc->writeRequest_uint8(sensor_id);
  this->_rpc->call(this->_handler_id, 0x11);

  type = this->_rpc->readResult_type(RPC_ARRAY);
  if(type != RPC_ARRAY) {
    return 0;
  }
  // ToDo: should be UINT8
  type = this->_rpc->readResult_raw_uint8();

  length = this->_rpc->readResult_raw_uint8();
  if(length > max_length) {
    length = max_length;
  }
  for(i = 0; i < length; i++) {
    data[i] = this->_rpc->readResult_raw_uint8();
  }
  return length;
}

uint16_t ArduRPC_SensorNodeRemote::getSensorType(uint8_t sensor_id)
{
  uint16_t value;

  this->_rpc->reset();
  this->_rpc->writeRequest_uint8(sensor_id);
  this->_rpc->call(this->_handler_id, 0x10);

  value = this->_rpc->readResult_uint16();
  return value;
}

uint8_t ArduRPC_SensorNodeRemote::setCredentials(char *uuid, char *key)
{
  uint16_t value;

  this->_rpc->reset();
  this->_rpc->writeRequest_string(uuid);
  this->_rpc->writeRequest_string(key);
  this->_rpc->call(this->_handler_id, 0x23);

  return 0;
}

uint8_t ArduRPC_SensorNodeRemote::setSensor(uint8_t sensor_id, uint16_t type_id, uint8_t *config, uint8_t config_length)
{
  uint8_t i;

  this->_rpc->reset();
  this->_rpc->writeRequest_uint8(sensor_id);
  this->_rpc->writeRequest_uint16(type_id);
  this->_rpc->writeRequest_uint8(config_length);
  for(i = 0; i < config_length; i++) {
    this->_rpc->writeRequest_uint8(config[i]);
  }
  this->_rpc->call(this->_handler_id, 0x12);

  return 0;
}


