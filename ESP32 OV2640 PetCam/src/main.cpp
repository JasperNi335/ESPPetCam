#include <Arduino.h>
#include "camera.h"

bool sendPhotoSerial();

void setup() {
  // sets delay for clean output
  delay(2000);

  // set baud rate
  Serial.begin(115200);
  Serial.println("Setup started\n"); // Debug print

  // setup camera
  if (!initCamera()){
    Serial.println("Failed to Initalise Camera\n");
  }

  setCameraSettings();
}

void loop() {
  if (sendPhotoSerial()) {Serial.println("photo taken and sent\n");
  }else {Serial.println("no photo taken\n");}

  Serial.println("Functioning\n");
  delay(5000);
}


bool sendPhotoSerial(){
  // try to capture image
  camera_fb_t* fb = cameraCapturePhoto();

  // if image is sucessfully captured
  if (fb){
      uint32_t size = fb->len;

      Serial.printf("Captured image size: %zu bytes\n", size);

      // sends image in smaller packages
      Serial.println("IMAGE_START");
      Serial.write((uint8_t*)&size, sizeof(size));

      // send in chunks of 64
      size_t sent = 0;
      while(sent < size){
        size_t chunk = 64;
        if (sent + chunk > size) chunk = size - sent;

        Serial.write(fb->buf + sent, chunk);

        sent += chunk;
        delay(1);
      }

      Serial.println("IMAGE_END");

      esp_camera_fb_return(fb);
      return true;
  }
  return false;
}

