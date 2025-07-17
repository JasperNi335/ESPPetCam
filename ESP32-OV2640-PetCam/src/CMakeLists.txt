idf_component_register(
    SRCS "main.cpp"
    REQUIRES MyLib bt nvs_flash esp_https_server esp_http_client esp_camera cJSON
)