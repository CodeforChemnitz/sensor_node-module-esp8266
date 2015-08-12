Proof of Concept
================



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
