#include "server.h"

static const char* TAG = "Server";

#define BOUNDARY_LENGTH 32
#define MJPEG_HEADER_SIZE 64
#define TARGET_FPS 1000
#define FRAME_DELAY 1000 / TARGET_FPS
#define JPEG_QUALITY 10 // uint8 0 - 255

// following MIME boundary allowed characters
const char BOUNDARY_CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

char PART_BOUNDARY[BOUNDARY_LENGTH + 1];
char _STREAM_CONTENT_TYPE[128];
char _STREAM_BOUNDARY[128];
char _STREAM_PART[] = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

const char* TEST_BOUNDARY = "frame";

void generateBoundary(char* buffer, size_t length){
    for (int i = 0; i < length - 1; i++){
        buffer[i] = BOUNDARY_CHARS[esp_random() % (sizeof(BOUNDARY_CHARS) - 1)];
    }
    buffer[length - 1] = '\0';
}

void generateTestBoundary(char* buffer){
    strcpy(buffer, TEST_BOUNDARY);
}

void setStreamConst(){
    generateTestBoundary(PART_BOUNDARY); // this line used for testing

    // dynamic boundaries to be implemeted later
    //generateBoundary(PART_BOUNDARY, sizeof(PART_BOUNDARY));

    ESP_LOGE(TAG, "Boundary is: %s", PART_BOUNDARY);

    snprintf(_STREAM_CONTENT_TYPE, sizeof(_STREAM_CONTENT_TYPE), "multipart/x-mixed-replace;boundary=%s", PART_BOUNDARY);
    snprintf(_STREAM_BOUNDARY, sizeof(_STREAM_BOUNDARY), "\r\n--%s\r\n", PART_BOUNDARY);
}

esp_err_t get_stream_handler(httpd_req_t* request) {
    // This handler will handle the esp32 stream
    ESP_LOGE(TAG, "Server recieved GET request");
    esp_err_t response = ESP_OK;

    camera_fb_t* fb = NULL;
    size_t jpg_buffer_length;
    uint8_t* jpg_buffer;
    char* header_buffer[MJPEG_HEADER_SIZE];

    response = httpd_resp_set_type(request, _STREAM_CONTENT_TYPE);
    if (response != ESP_OK){
        ESP_LOGE(TAG, "There was an error setting request type, Error: %s", esp_err_to_name(response));
        return response;
    }

    // main stream loop
    while(true){
        fb = cameraCapturePhoto();
        if (!fb){
            response = ESP_FAIL;
            break;
        }else{
            jpg_buffer = fb->buf;
            jpg_buffer_length = fb->len;
        }

        // sets the MJPEG format: Boundary -> Header -> Image
        if (response == ESP_OK){
            response = httpd_resp_send_chunk(request, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
            //ESP_LOGE(TAG, "%s", _STREAM_BOUNDARY); debug lines
        }
        if (response == ESP_OK){
            size_t header_len = snprintf((char*)header_buffer, MJPEG_HEADER_SIZE, _STREAM_PART, jpg_buffer_length);
            response = httpd_resp_send_chunk(request, (char*)header_buffer, header_len);
            //ESP_LOGE(TAG, "%s", header_buffer); debug lines
        }
        if (response == ESP_OK){
            response = httpd_resp_send_chunk(request, (char*)jpg_buffer, jpg_buffer_length);
        }

        // clean up buffers
        if(fb->format != PIXFORMAT_JPEG){
            free(jpg_buffer);
        }
        esp_camera_fb_return(fb);

        if (response != ESP_OK) break;

        // control frame rate manually, 
        //vTaskDelay(FRAME_DELAY / portTICK_PERIOD_MS);
    }

    return response;
}

httpd_uri_t uri_get = {
    .uri      = "/stream",
    .method   = HTTP_GET,
    .handler  = get_stream_handler,
    .user_ctx = NULL
};

httpd_handle_t server = NULL;

httpd_handle_t startServer(){
    setStreamConst();

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    esp_err_t err = httpd_start(&server, &config);
    if (err != ESP_OK){
        ESP_LOGE(TAG, "Failed to start server, Error: %s", esp_err_to_name(err));
    }else{
        httpd_register_uri_handler(server, &uri_get);
    }
    
    return server;
}

bool getServerStatus(){
    if (server) {
        ESP_LOGE(TAG, "Server is online");
        return true;
    }
    ESP_LOGE(TAG, "Server is not online");
    return false;
}

void endServer(){
    if (server) httpd_stop(server);
} 