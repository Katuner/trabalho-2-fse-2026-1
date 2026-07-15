#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "driver/gpio.h"
#include "driver/i2c.h"

#include "config.h"

static const char *TAG = "BIKE_COMPUTER";

// handles
static esp_mqtt_client_handle_t mqtt_client;
static QueueHandle_t xQueueSensorData;
static QueueHandle_t xQueueSpeedData;
static SemaphoreHandle_t xSemaphoreI2C;

typedef struct {
    float temperature;
    float pressure;
    float altitude;
    float accelX, accelY, accelZ;
} SensorData_t;

typedef struct {
    float speedKmh;
    float distanceKm;
    uint32_t wheelPulses;
} SpeedData_t;

// MQTT Event Handler
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            esp_mqtt_client_subscribe(mqtt_client, "v1/devices/me/rpc/request/+", 0);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            // Lógica de RPC para mudar tela ou resetar viagem
            if (strstr(event->data, "resetTrip")) {
                ESP_LOGI(TAG, "Resetting trip data via RPC");
            }
            break;
        default:
            break;
    }
}

// Task: Sensor de Velocidade (Interrupção)
static void IRAM_ATTR speed_isr_handler(void* arg) {
    static uint32_t last_time = 0;
    uint32_t now = xTaskGetTickCountFromISR();
    if (now - last_time > pdMS_TO_TICKS(DEBOUNCE_TIME_MS)) {
        // Notificar task de processamento
        last_time = now;
    }
}

// Task: Publicação MQTT
void MqttTask(void *pvParameters) {
    SensorData_t sensor;
    SpeedData_t speed;
    char telemetry[256];

    while (1) {
        if (xQueueReceive(xQueueSensorData, &sensor, pdMS_TO_TICKS(1000)) == pdTRUE) {
            snprintf(telemetry, sizeof(telemetry), 
                     "{\"temperature\":%.2f, \"pressure\":%.2f, \"altitude\":%.2f}", 
                     sensor.temperature, sensor.pressure, sensor.altitude);
            esp_mqtt_client_publish(mqtt_client, "v1/devices/me/telemetry", telemetry, 0, 1, 0);
        }
        
        if (xQueueReceive(xQueueSpeedData, &speed, 0) == pdTRUE) {
            snprintf(telemetry, sizeof(telemetry), 
                     "{\"speed\":%.2f, \"distance\":%.2f}", 
                     speed.speedKmh, speed.distanceKm);
            esp_mqtt_client_publish(mqtt_client, "v1/devices/me/telemetry", telemetry, 0, 1, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting Bike Computer...");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Configuração do MQTT
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URL,
        .credentials.username = MQTT_ACCESS_TOKEN,
    };
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);

    // Queues and Semaphores
    xQueueSensorData = xQueueCreate(5, sizeof(SensorData_t));
    xQueueSpeedData = xQueueCreate(5, sizeof(SpeedData_t));
    xSemaphoreI2C = xSemaphoreCreateMutex();

    // GPIO Init
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << PIN_SPEED_SENSOR),
        .pull_up_en = 1,
    };
    gpio_config(&io_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PIN_SPEED_SENSOR, speed_isr_handler, (void*) PIN_SPEED_SENSOR);

    // Create Tasks
    xTaskCreate(MqttTask, "MqttTask", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "System Initialized.");
}
