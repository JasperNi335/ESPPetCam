#include "client.h"
#include "network.h"

static const char* TAG = "HTTP";

bool pingServer(const char* url){
    // if not conncected to wifi dont 
    if (!isWiFiConnected()){
        ESP_LOGE("WiFi", "WiFi NOT Connected");
        return false;
    }

    // quickly check if the server is online via get method
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_GET,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

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
    // quick check for server status
    if (!pingServer(url)){
        ESP_LOGE(TAG, "Failed to ping server %s", url);
        return false;
    }

    // make client
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "java-app/json");
    esp_http_client_set_post_field(client, data, strlen(data));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK){
        ESP_LOGE(TAG, "POST to http server successful");\
        return true;
    }else{
        ESP_LOGE(TAG, "POST to http server failed");\
        return false;
    }
}

bool recieveFromServer(){}