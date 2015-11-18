#include "sensor_node.h"

void handleAPIHostname()
{
  uint8_t len;
  char hostname[NODE_EEPROM_API_HOSTNAME_MAX_LENGTH + 1];
  String hostname_s;
  if (server->method() == HTTP_GET) {
    len = getAPIHostname(&hostname[0], NODE_EEPROM_API_HOSTNAME_MAX_LENGTH);
    if(len > 0) {
      hostname[NODE_EEPROM_API_HOSTNAME_MAX_LENGTH] = '\0';
      server->send(200, "text/plain", hostname);
    } else {
      server->send(404, "text/plain", "Hostname not set");
    }
  } else if (server->method() == HTTP_POST) {
    if(server->args() == 0) {
      server->send(400, "text/plain", "No argument given");
      return;
    }
    hostname_s = server->arg("hostname");
    hostname_s.toCharArray(&hostname[0], NODE_EEPROM_API_HOSTNAME_MAX_LENGTH);
    hostname[NODE_EEPROM_API_HOSTNAME_MAX_LENGTH] = '\0';
    setAPIHostname(hostname, hostname_s.length());
    server->send(200, "text/plain", "Password set");
  }

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
    len = getWiFiPassword(&password[0], 64);
    if(len > 0) {
      password[64] = '\0';
      server->send(200, "text/plain", password);
    } else {
      server->send(404, "text/plain", "Password not set");
    }
  } else if (server->method() == HTTP_POST) {
    if(server->args() == 0) {
      server->send(400, "text/plain", "No argument given");
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

  String email = server->arg("email");
  String name = server->arg("name");

  // ToDo: read from eeprom
  char host[] = "192.168.0.1";
  int httpPort = 80;

  if(email.length() == 0) {
    server->send(400, "text/plain", "E-Mail not given");
    return;
  }

  String message = "";
  message += "email:" + email + "\r\n";
  if (name.length()) {
    message += "name:" + name + "\r\n";
  }

  if (!client->connect(host, httpPort)) {
    Serial.println("connection failed");
    server->send(500, "text/plain", "Unable to connect to remote server");
    return;
  }
  
  client->print("POST /sensors HTTP/1.1\r\n");
  client->print("Host: ");
  client->print(host);
  client->print("\r\n");
  client->print("X-Sensor-Version: 1\r\n");
  client->print("Content-Type: text/plain\r\n");
  client->print("Content-Length: ");
  client->print(message.length());
  client->print("\r\n");
  client->print("\r\n");
  client->print(message);

  String sensor_id = "";
  String sensor_key = "";

  unsigned long start_time = millis();
  while(1) {
    if(client->available()) {
      String line = client->readStringUntil('\n');
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
  client->stop();
  Serial.println(sensor_id);
  Serial.println(sensor_key);
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
  if (server->method() == HTTP_GET) {
    query = server->arg("q");
    int n = WiFi.scanNetworks();
    if (n == 0) {
      server->send(404, "text/plain", "[]");
    } else {
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
      server->send(200, "text/plain", data);
    }
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
    if(server->args() == 0) {
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

