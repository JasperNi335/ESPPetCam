#define CONFIG_ARDUHAL_LOG_DEFAULT_LEVEL ARDUHAL_LOG_LEVEL_INFO
#include <Arduino.h>
#include <WiFi.h>
#include "camera.h"
#include "network.h"
#include "keys.h"

void setup() {
  // sets delay for clean output
  delay(2000);

  // set baud rate
  Serial.begin(115200);

  Serial.println("Setup started\n"); // Debug print
  ESP_LOGE("TEST", "TESTING INFO LOG");

  // setup camera
  if (!initCamera()){
    Serial.println("Failed to Initalise Camera\n");
  }

  setCameraSettings();

  setupWiFi();
}

void loop() {
  /*
  if (sendPhotoSerial()) {Serial.println("photo taken and sent\n");
  }else {Serial.println("no photo taken\n");}

  Serial.println("Functioning\n");
  */

  isWiFiConnected();
  delay(5000);
}


