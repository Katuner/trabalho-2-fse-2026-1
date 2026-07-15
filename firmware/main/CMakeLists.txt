idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_mqtt driver esp_timer nvs_flash esp_wifi esp_event esp_netif)
