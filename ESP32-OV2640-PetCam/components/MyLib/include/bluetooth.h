#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"
#include "network.h"

// start spp bluetooth server
bool startBtServer();

// start spp bluetooth
bool startBt();

// checks server status from event handler
bool isBtServerOn();

// ends spp server 
void endBtServer();

#endif // BLUETOOTH_H