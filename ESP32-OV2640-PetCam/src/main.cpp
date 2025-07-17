#define CONFIG_ARDUHAL_LOG_DEFAULT_LEVEL ARDUHAL_LOG_LEVEL_INFO

#include "camera.h"
#include "network.h"
#include "client.h"
#include "server.h"
#include "bluetooth.h"
#include "keys.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "MAIN";

void setup() {
    ESP_LOGI(TAG, "Setup started");

    ESP_LOGI(TAG, "Initializing camera...");
    if (!initCamera()) {
        ESP_LOGE(TAG, "Failed to Initialize Camera");
        // Consider halting here to avoid reboot loop
        vTaskDelay(pdMS_TO_TICKS(10000));  // Wait so you can see error
        // or abort
        // esp_restart();
    } else {
        ESP_LOGI(TAG, "Camera initialized successfully");
    }

    ESP_LOGI(TAG, "Setting camera parameters");
    setCameraSettings();

    ESP_LOGI(TAG, "Setting up WiFi");
    setupWiFi();

    ESP_LOGI(TAG, "Starting server");
    startServer();

    ESP_LOGI(TAG, "Setup finished");
}


void loop() {
    /*
    camera_fb_t* fb = cameraCapturePhoto();
    if (fb){
        char* image = createImagePackage(fb);
        sendToServer(SERVER_URL, image);
        free(image);
    }
    */

    getServerStatus();

    vTaskDelay(pdMS_TO_TICKS(5000));  // Delay 5 seconds
}

extern "C" void app_main(void) {
    setup();

    xTaskCreate([](void*){
        while (true) {
            loop();
        }
    }, "main_loop_task", 4096, nullptr, 1, nullptr);
}


