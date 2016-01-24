#include "ArduRPC.h"
#include "sensor_node.h"
//#include <ESP8266mDNS.h>

// the serial port, it's also possible to use software serial
#define RPC_SERIAL_PORT Serial
#define RPC_SERIAL_BAUD 9600

#define RPC_NUM_HANDLERS  2
#define RPC_NUM_FUNCTIONS 0

uint8_t node_mode = 1;

ArduRPC *rpc;
ArduRPC_Serial *rpc_serial;
ArduRPC_SensorNode *rpc_sensor_node;
ArduRPC_SensorNodeRemote *sensor_remote;

ESP8266WebServer *server;

void setup() {
  pinMode(16, INPUT);
  pinMode(0, INPUT);
  pinMode(2, INPUT);
  uint8_t pin_mode;
  ArduRPCRequest *rpc_request;

  // Initialize the serial port
  RPC_SERIAL_PORT.begin(RPC_SERIAL_BAUD);

  NODE_DEBUG_PRINTLN("Setup");
  while(digitalRead(0) == HIGH) {
    delay(50);
  }

  node_mode = NODE_MODE_ACTIVE;

  pin_mode = digitalRead(2);
  if(pin_mode == HIGH) {
    NODE_DEBUG_PRINTLN("Set mode to config");
    node_mode = NODE_MODE_CONFIG;
  }

  EEPROM.begin(1024);

  if(node_mode == NODE_MODE_ACTIVE) {
    WiFi.mode(WIFI_STA);
    //connectWiFiClient(20);
    rpc = new ArduRPC(RPC_NUM_HANDLERS, RPC_NUM_FUNCTIONS);
    rpc_serial = new ArduRPC_Serial(RPC_SERIAL_PORT, *rpc);
    rpc_sensor_node = new ArduRPC_SensorNode(*rpc, "wifi");
  } else if (node_mode == NODE_MODE_CONFIG) {
    server = new ESP8266WebServer(80);
    rpc_request = new ArduRPCRequest();
    new ArduRPCRequest_Serial(*rpc_request, RPC_SERIAL_PORT);
    sensor_remote = new ArduRPC_SensorNodeRemote(*rpc_request, 0x00);

    initConfig();
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("SensorNode", "SensorNode");
    WiFi.softAPConfig(IPAddress(192, 168, 255, 254), IPAddress(), IPAddress(255, 255, 255, 0));
    connectWiFiClient(20);


    server->onNotFound(handleNotFound);

    server->begin();
    Serial.println("HTTP server started"); 
    Serial.print("Local: ");
    Serial.println(WiFi.localIP());
    Serial.print("AP: ");
    Serial.println(WiFi.softAPIP());
  }
}

void loop() {
  if (node_mode == NODE_MODE_ACTIVE) {
    while(1) {
      rpc_serial->readData();
    }
  } else if(node_mode == NODE_MODE_CONFIG) {
    server->handleClient();
  }
}

