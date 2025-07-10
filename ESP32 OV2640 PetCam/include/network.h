#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include "esp_wifi.h"
#include "keys.h"

void setupWiFi();
bool isWiFiConnected();

#endif // NETWORK_H