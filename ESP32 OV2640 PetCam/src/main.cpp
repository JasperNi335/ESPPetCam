#include <Arduino.h>
#include <WiFi.h>
#include "camera.h"
#include "keys.h"

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


