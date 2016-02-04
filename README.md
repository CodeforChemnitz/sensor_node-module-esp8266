WiFi Module for SensorNode
==========================

[![Build Status](https://travis-ci.org/CodeforChemnitz/sensor_node-module-esp8266.svg?branch=master)](https://travis-ci.org/CodeforChemnitz/sensor_node-module-esp8266)

Firmware for the WiFi Module for the SensorNode.

For more information have a look at [Code for Chemnitz: SensorNode project](http://codeforchemnitz.de/projects/sensor/)

Commands
========

Config
------

**SSID**

Set

```
curl --data "ssid=<ssid>" http://<ip|hostname>/config/wifi/sta/ssid
```

**WiFi password**

Set

```
curl --data "password=<password>" http://<ip|hostname>/config/wifi/sta/password
```

Actions
-------

**Register**

Register SensorNode at the Backendserver

```
curl --data "email=test@test.com" http://<ip|hostname>/action/register
```

**Save config**

```
curl http://<ip|hostname>/action/save
```

**Restart node**

```
curl http://<ip|hostname>/action/restart
```
