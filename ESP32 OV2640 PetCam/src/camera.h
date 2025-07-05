#ifndef CAMERA_H
#define CAMERA_H

#include <Arduino.h>
#include "esp_camera.h"

// Initilase the camera
bool initCamera();

// Capture a photo, Warning must release returned pointer
camera_fb_t* cameraCapturePhoto();

// deInitalise camera
void deInitCamera();

#endif // CAMERA_H