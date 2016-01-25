#include "sensor_node.h"

WiFiClient client;

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

bool connectSensorAPI()
{
  /* ToDo: Add SSL/TLS support */
  uint16_t port;
  char hostname[NODE_EEPROM_API_HOSTNAME_MAX_LENGTH + 1];

  if(WiFi.status() != WL_CONNECTED) {
    return false;
  }

  getAPIHostnameOrDefault(&hostname[0], NODE_EEPROM_API_HOSTNAME_MAX_LENGTH);
  hostname[NODE_EEPROM_API_HOSTNAME_MAX_LENGTH] = '\0';
  port = getAPIPortOrDefault();

  client.stop();
  if(client.connect(hostname, port)) {
    return true;
  }
  return false;
}
