#include "ArduRPC.h"
#include "sensor_node.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
//#include <ESP8266mDNS.h>

ESP8266WebServer server(80);

#define RPC_NUM_HANDLERS 20
#define RPC_NUM_FUNCTIONS 0

// the serial port, it's also possible to use software serial
#define RPC_SERIAL_PORT Serial
#define RPC_SERIAL_BAUD 9600

// Create a new rpc instance
ArduRPC rpc = ArduRPC(RPC_NUM_HANDLERS, RPC_NUM_FUNCTIONS);
// Create a new instance of the ArduRPC serial protocol handler
ArduRPC_Serial rpc_serial = ArduRPC_Serial(RPC_SERIAL_PORT, rpc);

// Create a new ArduRPC wrapper
SensorNodeESP8266 SensorNodeESP8288_Wrapper();

#define NODE_MODE_ACTIVE 0
#define NODE_MODE_CONFIG 1

#define NODE_EEPROM_SSID_OFFSET 1
#define NODE_EEPROM_PASSWORD_OFFSET 66

WiFiClient client;

uint8_t node_mode = 1;

void initConfig()
{
  if(getNodeConfigStatus()) {
    return;
  }
  EEPROM.write(NODE_EEPROM_SSID_OFFSET, 0x00);
  EEPROM.write(NODE_EEPROM_PASSWORD_OFFSET, 0x00);
  EEPROM.write(0, 0x33);
  EEPROM.commit();
}

void handleRoot()
{
  String message = "SensorNode\n\n";
  message += "SSID(AP): SensorNode\n";
  message += "IP(AP): ";
  message += String(WiFi.softAPIP());
  message += "\n\n";
  message += "SSID(local): ";
  message += "ToDo";
  message += "\nIP(local): ";
  message += String(WiFi.localIP());
  message += "\n";
  server.send(200, "text/plain", message);
}

void handleRegister()
{

  String email = server.arg("email");
  String name = server.arg("name");

  // ToDo: read from eeprom
  char host[] = "192.168.0.1";
  int httpPort = 80;

  if(email.length() == 0) {
    server.send(400, "text/plain", "E-Mail not given");
    return;
  }

  String message = "";
  message += "email:" + email + "\r\n";
  if (name.length()) {
    message += "name:" + name + "\r\n";
  }

  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    server.send(500, "text/plain", "Unable to connect to remote server");
    return;
  }
  
  client.print("POST /sensors HTTP/1.1\r\n");
  client.print("Host: ");
  client.print(host);
  client.print("\r\n");
  client.print("X-Sensor-Version: 1\r\n");
  client.print("Content-Type: text/plain\r\n");
  client.print("Content-Length: ");
  client.print(message.length());
  client.print("\r\n");
  client.print("\r\n");
  client.print(message);

  String sensor_id = "";
  String sensor_key = "";

  unsigned long start_time = millis();
  while(1) {
    if(client.available()) {
      String line = client.readStringUntil('\n');
      if(line.startsWith("apikey:")) {
        sensor_key = line.substring(7);
      }
      if(line.startsWith("id:")) {
        sensor_id = line.substring(3);
      }
      Serial.println(line);
    } else {
      delay(100);
    }
    if(sensor_id.length() > 0 && sensor_key.length() > 0) {
      break;
    }
    if(millis() - start_time > 10000) {
        break;
    }
  }
  client.stop();
  Serial.println(sensor_id);
  Serial.println(sensor_key);
  server.send(200, "text/plain", "Registred");
}

void handleRestart()
{
  ESP.restart();
}

void handleSave()
{
  EEPROM.commit();
  server.send(200, "text/plain", "Config saved to eeprom");
}

void handlePassword()
{
  uint8_t len;
  char password[65];
  String password_s;
  if (server.method() == HTTP_GET) {
    len = getWiFiPassword(&password[0], 64);
    if(len > 0) {
      password[64] = '\0';
      server.send(200, "text/plain", password);
    } else {
      server.send(404, "text/plain", "Password not set");
    }
  } else if (server.method() == HTTP_POST) {
    if(server.args() == 0) {
      server.send(400, "text/plain", "No argument given");
      return;
    }
    password_s = server.arg("password");
    password_s.toCharArray(&password[0], 64);
    password[64] = '\0';
    setWiFiPassword(password, password_s.length());
    server.send(200, "text/plain", "Password set");
  }

}

void handleSSID()
{
  uint8_t len;
  char ssid[65];
  String ssid_s;
  if (server.method() == HTTP_GET) {
    len = getWiFiSSID(&ssid[0], 64);
    if(len > 0) {
      ssid[64] = '\0';
      server.send(200, "text/plain", ssid);
    } else {
      server.send(404, "text/plain", "SSID not set");
    }
  } else if (server.method() == HTTP_POST) {
    if(server.args() == 0) {
      server.send(400, "text/plain", "No argument given");
      return;
    }
    ssid_s = server.arg("ssid");
    if(ssid_s.length() == 0) {
      server.send(400, "text/plain", "SSID must at least be 1 character long");
      return;
    }
    ssid_s.toCharArray(&ssid[0], 64);
    ssid[64] = '\0';
    setWiFiSSID(ssid, ssid_s.length());
    server.send(200, "text/plain", "SSID set");
  }

}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

uint8_t getNodeConfigStatus()
{
  uint8_t status;
  status = EEPROM.read(0);

  if(status != 0x33) {
    return 0;
  }
  return 1;
}

uint8_t getWiFiSSID(char *ssid, uint8_t max_len)
{
  uint8_t i, len;
  char c;

  if(!getNodeConfigStatus()) {
    return 0;
  }

  len = EEPROM.read(NODE_EEPROM_SSID_OFFSET);
  if(len > 64 || len == 0) {
    return 0;
  }

  if(len > max_len) {
    len = max_len;
  }

  for(i = 0; i < len; i++) {
    ssid[i] = EEPROM.read(NODE_EEPROM_SSID_OFFSET + i + 1);
  }

  if(len < max_len) {
    ssid[len] = '\0';
  }

  return len;
}

uint8_t getWiFiPassword(char *password, uint8_t max_len)
{
  uint8_t i, len;
  char c;

  if(!getNodeConfigStatus()) {
    return 0;
  }

  len = EEPROM.read(NODE_EEPROM_PASSWORD_OFFSET);
  if(len > 64 || len == 0) {
    return 0;
  }

  if(len > max_len) {
    len = max_len;
  }

  for(i = 0; i < len; i++) {
    password[i] = EEPROM.read(NODE_EEPROM_PASSWORD_OFFSET + i + 1);
  }

  if(len < max_len) {
    password[len] = '\0';
  }

  return len;
}

uint8_t setWiFiSSID(char *ssid, uint8_t len)
{
  uint8_t i;

  EEPROM.write(NODE_EEPROM_SSID_OFFSET, len);
  for(i = 0; i < len; i++) {
    EEPROM.write(NODE_EEPROM_SSID_OFFSET + i + 1, ssid[i]);
  }


  return len;
}

uint8_t setWiFiPassword(char *password, uint8_t len)
{
  uint8_t i;

  EEPROM.write(NODE_EEPROM_PASSWORD_OFFSET, len);
  for(i = 0; i < len; i++) {
    EEPROM.write(NODE_EEPROM_PASSWORD_OFFSET + i + 1, password[i]);
  }

  return len;
}

bool waitWiFiClientConnected(uint8_t);

bool connectWiFiClient(uint8_t connect_timeout=0)
{
  char ssid[65];
  char password[65];

  uint8_t len;

  len = getWiFiSSID(&ssid[0], 64);
  ssid[64] = '\0';
  Serial.println(ssid);
  if(len == 0) {
    Serial.println("exit");
    return false;
  }

  len = getWiFiPassword(&password[0], 64);
  password[64] = '\0';
  Serial.println(password);
  Serial.println(len);
  if(len == 0) {
    WiFi.begin(ssid);
  } else {
    WiFi.begin(ssid, password);
  }

  if(connect_timeout == 0) {
    return true;
  } else {
    return waitWiFiClientConnected(connect_timeout);
  }
}

bool waitWiFiClientConnected(uint8_t connect_timeout)
{
  uint8_t i;

  for(i = 0; i < connect_timeout * 2; i++) {  
    if(WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected");
      return true;
    }
    Serial.println("Waiting ...");
    delay(500);
  }
  return false;
}

void setup() {
  // Initialize the serial port
  RPC_SERIAL_PORT.begin(RPC_SERIAL_BAUD);

  EEPROM.begin(1024);

  if(node_mode == NODE_MODE_ACTIVE) {
    WiFi.mode(WIFI_STA);
  } else if (node_mode == NODE_MODE_CONFIG) {
    initConfig();
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("SensorNode");
    WiFi.softAPConfig(IPAddress(192, 168, 255, 254), IPAddress(), IPAddress(255, 255, 255, 0));
    connectWiFiClient(20);

    server.on("/", handleRoot);
    server.on("/action/register", handleRegister);
    server.on("/action/restart", handleRestart);
    server.on("/action/save", handleSave);
    server.on("/config/wifi/sta/ssid", handleSSID);
    server.on("/config/wifi/sta/password", handlePassword);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started"); 
  }


  Serial.print("Local: ");
  Serial.println(WiFi.localIP());
  Serial.print("AP: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  if (node_mode == NODE_MODE_ACTIVE) {
    // Process data
    rpc_serial.loop();
  } else if(node_mode == NODE_MODE_CONFIG) {
    server.handleClient();
  }
}
