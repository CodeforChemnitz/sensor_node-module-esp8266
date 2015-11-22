#include <ArduinoJson.h>
#include "sensor_node.h"

void handleAPIHostname()
{
  uint8_t len;
  char hostname[NODE_EEPROM_API_HOSTNAME_MAX_LENGTH + 1];
  String hostname_s;
  if (server->method() == HTTP_GET) {
    len = getAPIHostname(&hostname[0], NODE_EEPROM_API_HOSTNAME_MAX_LENGTH);
    hostname[NODE_EEPROM_API_HOSTNAME_MAX_LENGTH] = '\0';
    server->send(200, "text/plain", hostname);
  } else if (server->method() == HTTP_POST) {
    if(!server->hasArg("hostname")) {
      server->send(400, "text/plain", "No hostname given");
      return;
    }
    hostname_s = server->arg("hostname");
    hostname_s.toCharArray(&hostname[0], NODE_EEPROM_API_HOSTNAME_MAX_LENGTH);
    hostname[NODE_EEPROM_API_HOSTNAME_MAX_LENGTH] = '\0';
    setAPIHostname(hostname, hostname_s.length());
    server->send(200, "text/plain", "Hostname set");
  }
}

void handleAPIPort()
{
  uint16_t port;

  if (server->method() == HTTP_GET) {
    port = getAPIPort();
    server->send(200, "text/plain", String(port));
  } else if (server->method() == HTTP_POST) {
    if(!server->hasArg("port")) {
      server->send(400, "text/plain", "No port given");
      return;
    }
    port = server->arg("port").toInt();
    setAPIPort(port);
    server->send(200, "text/plain", "Port set");
  }
}

void handleInfoWiFiSTA()
{
  StaticJsonBuffer<128> jsonBuffer;
  char buffer[129];
  JsonObject& root = jsonBuffer.createObject();
  IPAddress tmp_ip;
  char tmp_buf_ip[16];
  char tmp_buf_netmask[16];

  if(WiFi.status() == WL_CONNECTED) {
    root["connected"] = true;
    tmp_ip = WiFi.localIP();
    sprintf(tmp_buf_ip, "%d.%d.%d.%d", tmp_ip[0], tmp_ip[1], tmp_ip[2], tmp_ip[3]);
    root["ip"] = tmp_buf_ip;

    tmp_ip = WiFi.subnetMask();
    sprintf(tmp_buf_netmask, "%d.%d.%d.%d", tmp_ip[0], tmp_ip[1], tmp_ip[2], tmp_ip[3]);
    root["netmask"] = tmp_buf_netmask;
  } else {
    root["connected"] = false;
  }

  root.printTo(buffer, 128);
  server->send(200, "application/json", buffer);
}

void handleNotFound(){

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server->uri();
  message += "\nMethod: ";
  message += (server->method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server->args();
  message += "\n";
  for (uint8_t i=0; i<server->args(); i++){
    message += " " + server->argName(i) + ": " + server->arg(i) + "\n";
  }
  server->send(404, "text/plain", message);
}

void handlePassword()
{
  uint8_t len;
  char password[65];
  String password_s;

  if (server->method() == HTTP_GET) {
    // Don't return the password for security reasons
    server->send(200, "text/plain", "");
  } else if (server->method() == HTTP_POST) {
    if(!server->hasArg("password")) {
      server->send(400, "text/plain", "No password given");
      return;
    }
    password_s = server->arg("password");
    password_s.toCharArray(&password[0], 64);
    password[64] = '\0';
    setWiFiPassword(password, password_s.length());
    server->send(200, "text/plain", "Password set");
  }

}

void handleRegister()
{
  WiFiClient *client;
  char hostname[NODE_EEPROM_API_HOSTNAME_MAX_LENGTH + 1];
  StaticJsonBuffer<200> jsonBuffer;
  char buffer[201];
  uint8_t i;

  String email = server->arg("email");
  String name = server->arg("name");

  if(email.length() == 0) {
    server->send(400, "text/plain", "E-Mail not given");
    return;
  }

  client = connectSensorAPI();

  if (client == NULL) {
    Serial.println("connection failed");
    server->send(500, "text/plain", "Unable to connect to remote server");
    return;
  }

  JsonObject& root = jsonBuffer.createObject();
  root["email"] = email;
  root["name"] = name;

  root.printTo(buffer, 200);

  getAPIHostnameOrDefault(&hostname[0], NODE_EEPROM_API_HOSTNAME_MAX_LENGTH);

  client->println("POST /sensors HTTP/1.1");
  client->print("Host: ");
  client->println(hostname);
  client->println("X-Sensor-Version: 1");
  client->println("Content-Type: application/json");
  client->print("Content-Length: ");
  client->println(strlen(buffer));
  client->println();
  client->print(buffer);

  String sensor_id = "";
  String sensor_key = "";

  unsigned long start_time = millis();
  i = 0;
  char c;
  while(1) {
    if(client->available()) {
      c = client->read();
      buffer[i] = c;
      i++;
      if(c == '}') {
        break;
      }
    } else {
      delay(100);
    }
    if(millis() - start_time > 5000) {
      server->send(400, "text/plain", "API Timeout");
      return;
    }
    if(i >= 200) {
      server->send(400, "text/plain", "Memory Limit");
      return;
    }
  }
  client->stop();
  JsonObject& root2 = jsonBuffer.parseObject(buffer);
  const char* uuid = root2["uuid"];
  const char* key = root2["key"];
  Serial.println(uuid);
  Serial.println(key);
  // ToDo: RPC call

  server->send(200, "text/plain", "Registred");
}

void handleRestart()
{
  ESP.restart();
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
  server->send(200, "text/plain", message);
}

void handleSave()
{
  EEPROM.commit();

  server->send(200, "text/plain", "Config saved to eeprom");
}

void handleScanSSID()
{
  String data;
  String query;
  String ssid;
  uint8_t first = 1;

  // reserve some memory
  data.reserve(600);

  if (server->method() == HTTP_GET) {
    query = server->arg("q");
    int n = WiFi.scanNetworks();
    data = "[";
    for (int i = 0; i < n; ++i) {
      ssid = WiFi.SSID(i);
      if(query.length() > 0 and !ssid.startsWith(query)) {
        continue;
      }
      if(first == 0) {
        data += ",";
      }
      first = 0;
      data += "{\"ssid\"=\"";
      data += ssid;
      //data += "\",rssi=\"";
      //data += WiFi.RSSI(i);
      data += "\",crypt=\"";
      if(WiFi.encryptionType(i) == ENC_TYPE_NONE) {
        data += "none";
      } else if (WiFi.encryptionType(i) == ENC_TYPE_WEP) {
        data += "wep";
      } else if (WiFi.encryptionType(i) == ENC_TYPE_TKIP) {
        data += "wpa";
      } else if (WiFi.encryptionType(i) == ENC_TYPE_CCMP) {
        data += "wpa2";
      }
      data += "\"}";
      delay(10);
      if(data.length() > 512) {
        break;
      }
    }
    data += "]";
    server->send(200, "application/json", data);
  }
}

void handleSSID()
{
  uint8_t len;
  char ssid[65];
  String ssid_s;

  if (server->method() == HTTP_GET) {
    len = getWiFiSSID(&ssid[0], 64);
    if(len > 0) {
      ssid[64] = '\0';
      server->send(200, "text/plain", ssid);
    } else {
      server->send(404, "text/plain", "SSID not set");
    }
  } else if (server->method() == HTTP_POST) {
    if(!server->hasArg("ssid")) {
      server->send(400, "text/plain", "No argument given");
      return;
    }
    ssid_s = server->arg("ssid");
    if(ssid_s.length() == 0) {
      server->send(400, "text/plain", "SSID must at least be 1 character long");
      return;
    }
    ssid_s.toCharArray(&ssid[0], 64);
    ssid[64] = '\0';
    setWiFiSSID(ssid, ssid_s.length());
    server->send(200, "text/plain", "SSID set");
  }

}

void submitFile(PGM_VOID_P mem, uint16_t size)
{
  uint16_t i, j;
  uint8_t buf[200];
  i = 0;
  while(i < size) {
    j = size - i;
    if(j > 200) {
      j = 200;
    }
    memcpy_P(buf, mem + i, j);
    server->client().write(&buf[0], j);
    i += 200;
  }
}
