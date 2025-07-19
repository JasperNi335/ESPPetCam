#include "bluetooth.h"

static const char* TAG = "Bluetooth";
static const char* BT_SERVER_NAME = "ESP_BT_Server";
static bool spp_server_running = false;

static char *bda2str(uint8_t * bda, char *str, size_t size){
    if (bda == NULL || str == NULL || size < 18) {
        return NULL;
    }

    uint8_t *p = bda;
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
            p[0], p[1], p[2], p[3], p[4], p[5]);
    return str;
}

// spp server status event handler
static void spp_status_handler(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    switch (event) {
        case ESP_SPP_START_EVT:
            spp_server_running = true;
            break;
        case ESP_SPP_CLOSE_EVT:
            spp_server_running = false;
            break;
        default:
            break;
    }
}

// Generic access profile event handler
static void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param){
    char bda_str[18];

    switch (event){
        case ESP_BT_GAP_DISC_RES_EVT:{ // 0
            ESP_LOGI(TAG, "Found device, bda:[%s]", bda2str(param->disc_res.bda, bda_str, sizeof(bda_str)));
            break;
        }
        case ESP_BT_GAP_AUTH_CMPL_EVT:{ // 4
            if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG, "authentication success: %s bda:[%s]", 
                        param->auth_cmpl.device_name,
                        bda2str(param->auth_cmpl.bda, bda_str, sizeof(bda_str)));
            } else {
                ESP_LOGE(TAG, "authentication failed, status:%d", param->auth_cmpl.stat);
            }
            break;
        }
        case ESP_BT_GAP_PIN_REQ_EVT:{ // 5
            ESP_LOGI(TAG, "ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d", param->pin_req.min_16_digit);
            if (param->pin_req.min_16_digit) {
                ESP_LOGE(TAG, "Input pin code: 0000 0000 0000 0000");
                esp_bt_pin_code_t pin_code = {0};
                esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
            } else {
                ESP_LOGI(TAG, "Input pin code: 1234");
                esp_bt_pin_code_t pin_code;
                pin_code[0] = '1';
                pin_code[1] = '2';
                pin_code[2] = '3';
                pin_code[3] = '4';
                esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
            }
            break;
        }
        case ESP_BT_GAP_CONFIG_EIR_DATA_EVT:{ // 10
             if (param->config_eir_data.stat == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG, "EIR data configured successfully.");
            } else {
                ESP_LOGE(TAG, "Failed to configure EIR data, status: %d", param->config_eir_data.stat);
            }
            break;
        }
        case ESP_BT_GAP_ACL_CONN_CMPL_STAT_EVT:{ // 16
            ESP_LOGI(TAG, "ACL connection complete: %s, status: %d",
                    bda2str(param->acl_conn_cmpl_stat.bda, bda_str, sizeof(bda_str)),
                    param->acl_conn_cmpl_stat.stat);
            break;
        }
        case ESP_BT_GAP_ACL_DISCONN_CMPL_STAT_EVT:{ // 17
            ESP_LOGI(TAG, "ACL disconnection complete: %s, reason: 0x%02X",
                    bda2str(param->acl_disconn_cmpl_stat.bda, bda_str, sizeof(bda_str)),
                    param->acl_disconn_cmpl_stat.reason & 0xFF);
            break;
        }
        case  ESP_BT_GAP_ENC_CHG_EVT:{ // 21
            ESP_LOGI(TAG, "Encryption status changed: %s, encrypted: %d",
                    bda2str(param->enc_chg.bda, bda_str, sizeof(bda_str)),
                    param->enc_chg.enc_mode);
            break;
        }
        default:
            ESP_LOGW(TAG, "Unhandled GAP event: %d", event);
            break;
    }
}

// Serial port protocol event handler
//------------------------------------------------------------------------------------------------------
// Future changes to make this accept wifi passwords in a json format from a python server
// JavaApp start -> JavaGUI + [wifi creds] -> python bt server -> esp32 spp acceptor -> inti wifi ...
//------------------------------------------------------------------------------------------------------
static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
    switch (event){
        case ESP_SPP_INIT_EVT:{
            ESP_LOGI(TAG, "SPP initialized");
            esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, BT_SERVER_NAME);
            break;
        }
        case ESP_SPP_START_EVT:{
            ESP_LOGI(TAG, "SPP server started");
            break;
        }
        case ESP_SPP_SRV_OPEN_EVT:{
            ESP_LOGI(TAG, "Client connected, handle: %lu", (unsigned long)param->srv_open.handle);
            break;
        }
        case ESP_SPP_DATA_IND_EVT:{
            ESP_LOGI(TAG, "Data received: %.*s", param->data_ind.len, param->data_ind.data);
            break;
        }
        case ESP_SPP_WRITE_EVT:{
            ESP_LOGI(TAG, "Data written");
            break;
        }
        case ESP_SPP_CLOSE_EVT:{
            ESP_LOGI(TAG, "Client disconnected");
            break;
        }
        default:
            ESP_LOGI(TAG, "Unhandled event: %d", event);
            break;
    }
}

bool startBtServer(){
    esp_err_t err = ESP_OK;
    if (!startBt()){
        ESP_LOGE(TAG, "Failed to initalise bluetooth");
        return false;
    }
    ESP_LOGI(TAG, "Initalised Bluetooth");
    if ((err = esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, BT_SERVER_NAME)) != ESP_OK) {
        ESP_LOGE(TAG, "SPP server start failed: %s", esp_err_to_name(err));
        return false;
    }

    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
    return true;
}

bool startBt(){
    esp_err_t err = ESP_OK;

    initNVS();

    esp_bt_controller_status_t status = esp_bt_controller_get_status();
    ESP_LOGI(TAG, "BT controller start status: %d", status);
    
    if (status == ESP_BT_CONTROLLER_STATUS_ENABLED) {
        esp_bt_controller_disable();
    }
    if (status == ESP_BT_CONTROLLER_STATUS_INITED) {
        esp_bt_controller_deinit();
    }

    esp_bt_controller_config_t config = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    if ((err = esp_bt_controller_init(&config)) != ESP_OK){
        ESP_LOGE(TAG, "Error intialising bluetooth, Error: %s", esp_err_to_name(err));
        return false;
    }

    status = esp_bt_controller_get_status();
    ESP_LOGI(TAG, "BT controller status: %d", status);

    if ((err = esp_bt_controller_enable(BTDM_CONTROLLER_MODE_EFF)) != ESP_OK){
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
        ESP_LOGE(TAG, "%s gap register failed: %s", __func__, esp_err_to_name(err));
        return false;
    }

    if ((err = esp_spp_register_callback(esp_spp_cb)) != ESP_OK) {
        ESP_LOGE(TAG, "%s spp register failed: %s", __func__, esp_err_to_name(err));
        return false;
    }

    esp_bt_gap_set_device_name(BT_SERVER_NAME);

    esp_spp_cfg_t bt_spp_cfg = {
        .mode = ESP_SPP_MODE_CB,
        .enable_l2cap_ertm = true,
        .tx_buffer_size = 0,
    };

    if ((err = esp_spp_enhanced_init(&bt_spp_cfg)) != ESP_OK) {
        ESP_LOGE(TAG, "%s spp init failed: %s", __func__, esp_err_to_name(err));
        return false;
    }

    return true;
}

bool getBtServerStatus(){
    return spp_server_running;
}

void endBtServer(){
    esp_err_t err = esp_spp_deinit();
    if (err != ESP_OK){
        ESP_LOGE(TAG, "Failed to deinit esp spp, Error: %s", esp_err_to_name(err));
    }

    esp_bt_controller_status_t status = esp_bt_controller_get_status();
    ESP_LOGI(TAG, "BT controller status: %d", status);
    
    if (status == ESP_BT_CONTROLLER_STATUS_ENABLED) {
        esp_bt_controller_disable();
    }
    if (status == ESP_BT_CONTROLLER_STATUS_INITED) {
        esp_bt_controller_deinit();
    }
    ESP_LOGI(TAG, "Ended bluetooth server");
}