#ifndef CLIENT_H
#define CLIENT_H

#include <Arduino.h>
#include "esp_http_client.h"

// check if there is a connection
bool pingServer(const char* url);

// send data via http/s
bool sendToServer(const char* url, const char* data);

// recieve data via http/s
bool recieveFromServer(const char* url);

#endif // CLIENT_H