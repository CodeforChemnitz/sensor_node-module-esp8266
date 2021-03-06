#include <ArduinoJson.h>
#include "sensor_node.h"
#include "sensor_node_file.h"

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

void handleConfigSensor(int sensor_id)
{
  uint8_t i, j;
  uint16_t type_id;
  uint8_t count;
  uint8_t buf[32];
  uint8_t result_length;
  uint16_t i2, j2;
  DataString<1024> output;
  StaticJsonBuffer<512> jsonBuffer;

  count = sensor_remote->getMaxSensorCount();
  if(count < sensor_id || sensor_id < 0) {
    server->send(404, "application/json", "{}");
    return;
  }

  if(server->method() == HTTP_GET) {
    output.print("{");
    output.print("\"type\":");
    type_id = sensor_remote->getSensorType(sensor_id);
    output.print(type_id);
    output.print(",");
    output.print("\"config\":[");
    result_length = sensor_remote->getSensorConfig(sensor_id, &buf[0], sizeof(buf));
    for(j = 0; j < result_length; j++) {
      if(j > 0) {
        output.print(",");
      }
      output.print(buf[j]);
    }
    output.print("]");
    output.print("}");

    server->setContentLength(output.length);
    server->send(200, "application/json", "");

    i2 = 0;
    while(i2 < output.length) {
      j2 = output.length - i2;
      if(j2 > 200) {
        j2 = 200;
      }
      server->client().write(&output.data[i2], j2);
      i2 += 200;
    }
    return;
  }

  if(!server->hasArg("config")) {
    server->send(400, "text/plain", "No config given");
    return;
  }
  JsonObject& root = jsonBuffer.parseObject(server->arg("config"));
  type_id = root["type"];
  JsonArray& configArray = root["config"];
  j = configArray.size();
  if(j > sizeof(buf)) {
    j = sizeof(buf);
  }
  for(i = 0; i < j; i++) {
    buf[i] = configArray.get<uint8_t>(i);
  }
  sensor_remote->setSensor(sensor_id, type_id, &buf[0], j);
  server->send(200, "text/plain", "Success");
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

void handleNotFound()
{
  int sensor_id;
  String uri = server->uri();
  String tmp;

  if (uri.equals("/")) {
    handleRoot();
    return;
  }
  if (uri.equals("/action/register")) {
    handleRegister();
    return;
  }
  if (uri.equals("/action/restart")) {
    handleRestart();
    return;
  }
  if (uri.equals("/action/save")) {
    handleSave();
    return;
  }
  if (uri.equals("/config/api/hostname")) {
    handleAPIHostname();
    return;
  }
  if (uri.equals("/config/api/port")) {
    handleAPIPort();
    return;
  }
  if (uri.startsWith("/config/sensor/")) {
    tmp = uri.substring(15, 19);
    sensor_id = tmp.toInt();
    handleConfigSensor(sensor_id);
    return;
  }
  if (uri.equals("/config/wifi/sta/ssid")) {
    handleSSID();
    return;
  }
  if (uri.equals("/config/wifi/sta/password")) {
    handlePassword();
    return;
  }
  if (uri.equals("/info/wifi/ssids")) {
    handleScanSSID();
    return;
  }
  if (uri.equals("/info/wifi/sta")) {
    handleInfoWiFiSTA();
    return;
  }
  if (uri.equals("/setup")) {
    server->setContentLength(sizeof(PAGE_setup));
    server->sendHeader("Content-Encoding", "gzip");
    server->send(200, "text/html", "");
    submitFile(PAGE_setup, sizeof(PAGE_setup));
    return;
  }
  if (uri.equals("/setup/js.js")) {
    server->setContentLength(sizeof(FILE_js));
    server->sendHeader("Content-Encoding", "gzip");
    server->send(200, "application/javascript", "");
    submitFile(FILE_js, sizeof(FILE_js));
    return;
  }
  
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

  if (connectSensorAPI() == false) {
    Serial.println("connection failed");
    server->send(500, "text/plain", "Unable to connect to remote server");
    return;
  }

  JsonObject& root = jsonBuffer.createObject();
  root["email"] = email;
  root["name"] = name;

  //root.printTo(buffer, 200);

  getAPIHostnameOrDefault(&hostname[0], NODE_EEPROM_API_HOSTNAME_MAX_LENGTH);

  client.println("POST /sensors HTTP/1.1");
  client.print("Host: ");
  client.println(hostname);
  client.println("X-Sensor-Version: 1");
  client.println("Content-Type: application/json");
  client.print("Content-Length: ");
  //client.println(strlen(buffer));
  client.println(root.measureLength());
  client.println();
  root.printTo(client);
  //client.print(buffer);

  unsigned long start_time = millis();
  char c;
  int code;

  // ToDo:
  client.find(' ');
  code = client.parseInt();
  NODE_DEBUG_PRINT("http-code ");
  NODE_DEBUG_PRINTLN(code);
  if (code != 200) {
    server->send(400, "text/plain", "Remote error");
    return;
  }

  i = 0;
  client.find("\r\n\r\n");
  while(1) {
    if(client.available()) {
      c = client.read();
      buffer[i] = c;
      i++;
      if(c == '}') {
        break;
      }
      if(i >= sizeof(buffer) - 2) {
        server->send(400, "text/plain", "Memory Limit");
        return;
      }
    } else {
      delay(100);
    }
    if(millis() - start_time > 5000) {
      server->send(400, "text/plain", "API Timeout");
      return;
    }
  }
  buffer[i] = '\0';
  Serial.println(buffer);

  JsonObject& root2 = jsonBuffer.parseObject(buffer);
  if(!root2.success() || !root2.containsKey("id") || !root2.containsKey("key")) {
    server->send(400, "text/plain", "Error parsing registration data");
    return;
  }

  const char *ptr_uuid = root2["id"];
  const char *ptr_key = root2["key"];
  char uuid[64] = {0};
  char key[64];
  strncpy(&uuid[0], ptr_uuid, sizeof(uuid));
  strncpy(&key[0], ptr_key, sizeof(key));

  Serial.println(uuid);
  Serial.println(key);

  // ToDo: RPC call
  sensor_remote->setCredentials(uuid, key);

  server->send(200, "text/plain", "Registred");
}

void handleRestart()
{
  ESP.restart();
}

void handleRoot()
{
  IPAddress tmp_ip;
  char tmp_buf[16];

  String message = "SensorNode\n\n";
  message += "SSID(AP): SensorNode\n";
  message += "IP(AP): ";
  message += String(WiFi.softAPIP());
  message += "\n\n";
  message += "SSID(local): ";
  message += "ToDo";

  if(WiFi.status() == WL_CONNECTED) {
    message += "\nIP(local): ";
    tmp_ip = WiFi.localIP();
    sprintf(tmp_buf, "%d.%d.%d.%d", tmp_ip[0], tmp_ip[1], tmp_ip[2], tmp_ip[3]);
    message += tmp_buf;
    message += "\nSubnet Mask(local): ";
    tmp_ip = WiFi.subnetMask();
    sprintf(tmp_buf, "%d.%d.%d.%d", tmp_ip[0], tmp_ip[1], tmp_ip[2], tmp_ip[3]);
    message += tmp_buf;
  }
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
      server->send(400, "text/plain", "No ssid given");
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
