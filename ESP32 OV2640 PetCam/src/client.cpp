#include "client.h"
#include "network.h"

esp_http_client_config_t config = {};
esp_http_client_handle_t client = nullptr;

static const char* TAG = "HTTP";

bool pingServer(const char* url){
    // if not conncected to wifi dont 
    if (!isWiFiConnected()){
        ESP_LOGE("WiFi", "WiFi NOT Connected");
        return false;
    }

    // quickly check if the server is online via get method
    config.url = url;
    config.method = HTTP_METHOD_GET;
    client = esp_http_client_init(&config);

    if (!client){
        ESP_LOGE(TAG, "Failed to initalise http client");
        return false;
    }

    // pings the server and checks response
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK){
        ESP_LOGE(TAG, "Conncection to http server successful");\
        return true;
    }else{
        ESP_LOGE(TAG, "Conncection to http server failed");\
        return false;
    }
}

bool sendToServer(const char* url, const char* data){


}

bool recieveFromServer(){}