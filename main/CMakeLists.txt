idf_component_register(SRCS "main.c"
                    PRIV_INCLUDE_DIRS .
                        PRIV_REQUIRES nvs_flash esp_psram esp_wifi esp_http_server esp_netif lwip)
