#ifndef CLIENT_H
#define CLIENT_H

#include <Arduino.h>
#include "esp_http_client.h"
#include "cJson.h"
#include "camera.h"
#include "network.h"

// check if there is a connection
bool pingServer(const char* url);

// send data via http/s
bool sendToServer(const char* url, const char* data);

// recieve data via http/s
bool recieveFromServer(const char* url);

// turn image into json package
// caller must free the created pointer/returned pointer
char* createImagePackage(camera_fb_t* image_pointer);

#endif // CLIENT_H