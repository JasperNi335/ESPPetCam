idf_component_register(
    SRCS "src/bluetooth.cpp"
         "src/camera.cpp"
         "src/client.cpp"
         "src/network.cpp"
         "src/server.cpp"
    INCLUDE_DIRS "include"
    REQUIRES bt nvs_flash esp_https_server esp_http_client esp_camera cJSON
)
