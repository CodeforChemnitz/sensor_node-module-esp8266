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

ESP8266WebServer *server;
WiFiClient *client;

void setup() {
  pinMode(16, INPUT);
  pinMode(0, INPUT);
  pinMode(2, INPUT);
  uint8_t pin_mode;

  while(digitalRead(0) == HIGH) {
    delay(50);
  }

  pin_mode = digitalRead(2); 

  node_mode = NODE_MODE_ACTIVE;

  if(pin_mode == HIGH) {
    node_mode = NODE_MODE_CONFIG;
  }

  // Initialize the serial port
  RPC_SERIAL_PORT.begin(RPC_SERIAL_BAUD);

  EEPROM.begin(1024);

  client = new WiFiClient();

  if(node_mode == NODE_MODE_ACTIVE) {
    WiFi.mode(WIFI_STA);
    //connectWiFiClient(20);
    rpc = new ArduRPC(RPC_NUM_HANDLERS, RPC_NUM_FUNCTIONS);
    rpc_serial = new ArduRPC_Serial(RPC_SERIAL_PORT, *rpc);
    rpc_sensor_node = new ArduRPC_SensorNode(*rpc, "wifi");
  } else if (node_mode == NODE_MODE_CONFIG) {
    server = new ESP8266WebServer(80);

    initConfig();
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("SensorNode");
    WiFi.softAPConfig(IPAddress(192, 168, 255, 254), IPAddress(), IPAddress(255, 255, 255, 0));
    connectWiFiClient(20);


    server->on("/", handleRoot);
    server->on("/action/register", handleRegister);
    server->on("/action/restart", handleRestart);
    server->on("/action/save", handleSave);
    server->on("/action/wifi/ssids", handleScanSSID);
    server->on("/config/wifi/sta/ssid", handleSSID);
    server->on("/config/wifi/sta/password", handlePassword);
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

