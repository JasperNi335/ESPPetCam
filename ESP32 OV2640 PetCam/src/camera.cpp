#include "camera.h"

#define PWDN_GPIO_NUM  32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  0
#define SIOD_GPIO_NUM  26
#define SIOC_GPIO_NUM  27
#define Y9_GPIO_NUM    35
#define Y8_GPIO_NUM    34
#define Y7_GPIO_NUM    39
#define Y6_GPIO_NUM    36
#define Y5_GPIO_NUM    21
#define Y4_GPIO_NUM    19
#define Y3_GPIO_NUM    18
#define Y2_GPIO_NUM    5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM  23
#define PCLK_GPIO_NUM  22

const char* TAG = "CAMERA";

camera_config_t camera_config = {
    .pin_pwdn       = PWDN_GPIO_NUM,
    .pin_reset      = RESET_GPIO_NUM,
    .pin_xclk       = XCLK_GPIO_NUM,
    .pin_sccb_sda   = SIOD_GPIO_NUM,
    .pin_sccb_scl   = SIOC_GPIO_NUM,
  
    .pin_d7         = Y9_GPIO_NUM,
    .pin_d6         = Y8_GPIO_NUM,
    .pin_d5         = Y7_GPIO_NUM,
    .pin_d4         = Y6_GPIO_NUM,
    .pin_d3         = Y5_GPIO_NUM,
    .pin_d2         = Y4_GPIO_NUM,
    .pin_d1         = Y3_GPIO_NUM,
    .pin_d0         = Y2_GPIO_NUM,
    .pin_vsync      = VSYNC_GPIO_NUM,
    .pin_href       = HREF_GPIO_NUM,
    .pin_pclk       = PCLK_GPIO_NUM,
  
    .xclk_freq_hz   = 20000000,
    .ledc_timer     = LEDC_TIMER_0,
    .ledc_channel   = LEDC_CHANNEL_0,
    .pixel_format   = PIXFORMAT_JPEG,
    .frame_size     = FRAMESIZE_SVGA,
    .jpeg_quality   = 4,
    .fb_count       = 1
};

/*-----------------------------------------
Default Camera methods
-----------------------------------------*/

bool initCamera(){
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        Serial.printf("Camera Init Failed\n Error: 0x%x\n", err);
        return false;
    }
    Serial.printf("Camera Init Successful\n");

    // take a few starting photos to fix green tint
    for (int i = 0; i < 100; i++){
        camera_fb_t* fb = cameraCapturePhoto();

        if (fb) esp_camera_fb_return(fb);
    }
    return true;
}

void setCameraSettings(){
    sensor_t * s = esp_camera_sensor_get();

    if (!s){
        Serial.printf("Camera not Found\n");
    }

    s->set_brightness(s, 0);     // -2 to 2
    s->set_contrast(s, 0);       // -2 to 2
    s->set_saturation(s, 0);     // -2 to 2
    s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
    s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
    s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
    s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    s->set_exposure_ctrl(s, 0);  // 0 = disable , 1 = enable
    s->set_aec2(s, 0);           // 0 = disable , 1 = enable
    s->set_ae_level(s, 0);       // -2 to 2
    s->set_aec_value(s, 300);    // 0 to 1200
    s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
    s->set_agc_gain(s, 0);       // 0 to 30
    s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
    s->set_bpc(s, 0);            // 0 = disable , 1 = enable
    s->set_wpc(s, 1);            // 0 = disable , 1 = enable
    s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
    s->set_lenc(s, 1);           // 0 = disable , 1 = enable
    s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
    s->set_vflip(s, 0);          // 0 = disable , 1 = enable
    s->set_dcw(s, 1);            // 0 = disable , 1 = enable
    s->set_colorbar(s, 0);       // 0 = disable , 1 = enable

    Serial.printf("Camera Settings applied\n");
}

camera_fb_t* cameraCapturePhoto(){
    camera_fb_t* fb = esp_camera_fb_get();
    // if failed to capturn image
    if (!fb){
        Serial.printf("Camera Capture Failed\n");
        return nullptr;
    }
    return fb;
}

// loads the buffer from the camera to a base 64 format
// caller must free the buffer after use
char* fb_to_b64(camera_fb_t* frame_buffer){
    if (!frame_buffer || !frame_buffer->buf || frame_buffer->len == 0){
        ESP_LOGE(TAG, "Empty or Null frame buffer");
        return nullptr;
    }

    // turns 3 bytes to 4 bytes
    // plus 2 is for rounding up
    size_t b64_len = 4 * ((frame_buffer->len + 2) / 3);
    char* output_b64_buffer = (char*)malloc(b64_len + 1);

    if (!output_b64_buffer){
        ESP_LOGE(TAG, "Failed to allocate memory");
        return nullptr;
    }

    size_t output_len = 0;
    esp_err_t err = mbedtls_base64_encode((unsigned char*)output_b64_buffer, 
                                          b64_len + 1,
                                          &output_len, 
                                          frame_buffer->buf, 
                                          frame_buffer->len);

    if (err != ESP_OK){
        ESP_LOGE("Encode", "Encoding failed, Error: %s", err);
        free(output_b64_buffer);
        return nullptr;
    }

    output_b64_buffer[output_len] = '\0';
    return output_b64_buffer;
}

void deInitCamera(){
    esp_camera_deinit();
}

/*-----------------------------------------
Serial Camera methods
-----------------------------------------*/

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

/*-----------------------------------------
Wifi Camera methods
-----------------------------------------*/

