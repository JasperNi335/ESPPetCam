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
    vTaskDelay(pdMS_TO_TICKS(3000));
    esp_log_level_set("*", ESP_LOG_INFO);

    ESP_LOGE("TEST", "This is an ERROR log");
    ESP_LOGW("TEST", "This is a WARNING log");
    ESP_LOGI("TEST", "This is an INFO log");
    // ESP_LOGD("TEST", "This is a DEBUG log");
    // ESP_LOGV("TEST", "This is a VERBOSE log");

    ESP_LOGI(TAG, "Setup started");

    ESP_LOGI(TAG, "Initializing camera...");
    if (!initCamera()) {
        ESP_LOGE(TAG, "Failed to Initialize Camera");
        // Consider halting here to avoid reboot loop
        vTaskDelay(pdMS_TO_TICKS(10000));  // Wait so you can see error
    } else {
        ESP_LOGI(TAG, "Camera initialized successfully");
    }

    ESP_LOGI(TAG, "Setting camera parameters");
    setCameraSettings();
    /*
    ESP_LOGI(TAG, "Setting up WiFi");
    setupWiFi();

    ESP_LOGI(TAG, "Starting http server");
    startServer();*/

    ESP_LOGI(TAG, "Starting bluetooth server");
    startBtServer();

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


