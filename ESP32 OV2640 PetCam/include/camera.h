#ifndef CAMERA_H
#define CAMERA_H

#include <Arduino.h>
#include "esp_camera.h"


/*-----------------------------------------
Default Camera methods
-----------------------------------------*/

// Initilase the camera
bool initCamera();

// set camera settings
void setCameraSettings();

// Capture a photo, Warning must release returned pointer
camera_fb_t* cameraCapturePhoto();

// deInitalise camera
void deInitCamera();

/*-----------------------------------------
Serial Camera methods
-----------------------------------------*/

// Serial Communication for camera
bool sendPhotoSerial();

/*-----------------------------------------
Wifi Camera methods
-----------------------------------------*/

#endif // CAMERA_H