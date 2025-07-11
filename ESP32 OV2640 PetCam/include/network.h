#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include <String.h>
#include "esp_wifi.h"
#include "keys.h"
#include "network.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

void setupWiFi();

bool isWiFiConnected();

#endif // NETWORK_H