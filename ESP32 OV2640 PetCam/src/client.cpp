#include "client.h"
#include "network.h"

static const char* TAG = "HTTP";

static esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        // more behaviour to be added
        case HTTP_EVENT_ON_DATA: // temporary data handling
            if (!esp_http_client_is_chunked_response(evt->client)) {
                char* response = strndup((const char*)evt->data, evt->data_len);
                ESP_LOGE(TAG, "Response body: %s", response);
                free(response);
            }
            break;
        default:
            // default behaviour to be added
            break;
    }
    return ESP_OK;
}

bool pingServer(const char* url){
    if (!isWiFiConnected()) return false;

    // quickly check if the server is online via get method
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_GET,
        .timeout_ms = 5000,
        .event_handler = _http_event_handler,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "Failed to initialise http client");
        return false;
    }

    // pings the server and checks response
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        ESP_LOGE(TAG, "Status code: %d", status_code);
        esp_http_client_cleanup(client);
        return true;
    } else {
        ESP_LOGE(TAG, "Request failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return false;
    }
}

bool sendToServer(const char* url, const char* data){
    // quick check for server status
    if (!pingServer(url)) return false;

    // make client
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK){
        ESP_LOGE(TAG, "POST to http server successful");
        esp_http_client_cleanup(client);
        return true;
    }else{
        ESP_LOGE(TAG, "POST to http server failed");
        ESP_LOGE(TAG, "Error: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return false;
    }
}

bool recieveFromServer(const char* url){
    // quick check for server status
    if (!pingServer(url)) return false;

    // make client
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_GET,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK){
        ESP_LOGE(TAG, "GET from http server successful");

        char buffer[256];
        int content_length = esp_http_client_read_response(client, buffer, sizeof(buffer) - 1); 
        if (content_length >= 0){
            buffer[content_length] = '\0'; // null termination
            ESP_LOGE(TAG, "Server response : %s", buffer);

            // manage json package from server 
            // -- implement this later --
        }else{
            ESP_LOGE(TAG, "Failed to read response");
            return false;
        }
        esp_http_client_cleanup(client);
        return true;
    }else{
        ESP_LOGE(TAG, "GET from http server failed");
        ESP_LOGE(TAG, "Error: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return false;
    }
}