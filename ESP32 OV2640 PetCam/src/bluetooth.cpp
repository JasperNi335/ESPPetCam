#include "bluetooth.h"

static const char* TAG = "Bluetooth";
static const char* BT_SERVER_NAME = "ESP_BT_Server";

static char *bda2str(uint8_t * bda, char *str, size_t size){
    if (bda == NULL || str == NULL || size < 18) {
        return NULL;
    }

    uint8_t *p = bda;
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
            p[0], p[1], p[2], p[3], p[4], p[5]);
    return str;
}

// Generic access profile event handler
static void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param){
    char bda_str[18];

    switch (event){
        case ESP_BT_GAP_AUTH_CMPL_EVT:{
            if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(TAG, "authentication success: %s bda:[%s]", 
                        param->auth_cmpl.device_name,
                        bda2str(param->auth_cmpl.bda, bda_str, 
                        sizeof(bda_str)));
            } else {
                ESP_LOGE(TAG, "authentication failed, status:%d", param->auth_cmpl.stat);
            }
            break;
        }
        case ESP_BT_GAP_PIN_REQ_EVT:{
            ESP_LOGE(TAG, "ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d", param->pin_req.min_16_digit);
            if (param->pin_req.min_16_digit) {
                ESP_LOGE(TAG, "Input pin code: 0000 0000 0000 0000");
                esp_bt_pin_code_t pin_code = {0};
                esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
            } else {
                ESP_LOGI(SPP_TAG, "Input pin code: 1234");
                esp_bt_pin_code_t pin_code;
                pin_code[0] = '1';
                pin_code[1] = '2';
                pin_code[2] = '3';
                pin_code[3] = '4';
                esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
            }
            break;
        }
    }
}

// Serial port protocol event hanlder
static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
    char bda_str[18];

    switch (event){
        case ESP_SPP_INIT_EVT:{
            ESP_LOGE(TAG, "SPP initialized");
            esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, BT_SERVER_NAME);
            break;
        }
        case ESP_SPP_START_EVT:{
            ESP_LOGE(TAG, "SPP server started");
            break;
        }
        case ESP_SPP_SRV_OPEN_EVT:{
            ESP_LOGE(TAG, "Client connected");
            break;
        }
        case ESP_SPP_DATA_IND_EVT:{
            ESP_LOGE(TAG, "Data received: %.*s", param->data_ind.len, param->data_ind.data);
            break;
        }
        case ESP_SPP_WRITE_EVT:{
            ESP_LOGE(TAG, "Data written");
            break;
        }
        case ESP_SPP_CLOSE_EVT:{
            ESP_LOGE(TAG, "Client disconnected");
            break;
        }
        default:
            ESP_LOGE(TAG, "Unhandled event: %d", event);
            break;
    }
}

bool startBtServer(){
    esp_err_t err = ESP_OK;

    initNVS();

    esp_err_t release_err = esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
    if (release_err != ESP_OK && release_err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "BLE memory release failed: %s", esp_err_to_name(release_err));
        return false;
    }

    esp_bt_controller_config_t config  = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    if ((err = esp_bt_controller_init(&config)) != ESP_OK){
        ESP_LOGE(TAG, "Error intialising bluetooth, Error: %s", esp_err_to_name(err));
        return false;
    }

    if ((err = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK){
        ESP_LOGE(TAG, "Error enabling classic bluetooth, Error: %s", esp_err_to_name(err));
        return false;
    }

    // Initialize and enable Bluedroid stack
    if ((err = esp_bluedroid_init()) != ESP_OK) {
        ESP_LOGE(TAG, "Error initializing bluedroid, Error: %s", esp_err_to_name(err));
        return false;
    }
    if ((err = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(TAG, "Error enabling bluedroid, Error: %s", esp_err_to_name(err));
        return false;
    }

    if ((err = esp_bt_gap_register_callback(esp_bt_gap_cb)) != ESP_OK) {
        ESP_LOGE(TAG, "%s gap register failed: %s", esp_err_to_name(err));
        return false;
    }

    if ((err = esp_spp_register_callback(esp_spp_cb)) != ESP_OK) {
        ESP_LOGE(TAG, "%s spp register failed: %s", esp_err_to_name(err));
        return false;
    }

    esp_bt_dev_set_device_name(BT_SERVER_NAME);
    esp_spp_init(ESP_SPP_MODE_CB);

    return true;
}