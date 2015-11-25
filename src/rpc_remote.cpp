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

uint16_t ArduRPC_SensorNodeRemote::getSensorType(uint8_t sensor_id)
{
  uint8_t value;

  this->_rpc->reset();
  this->_rpc->writeRequest_uint8(sensor_id);
  this->_rpc->call(this->_handler_id, 0x10);

  value = this->_rpc->readResult_uint16();
  return value;
}

