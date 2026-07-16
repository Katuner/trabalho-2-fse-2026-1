#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

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

// Handles globais
static esp_mqtt_client_handle_t mqtt_client;
static QueueHandle_t xQueueSensorData;
static QueueHandle_t xQueueSpeedData;
static SemaphoreHandle_t xSemaphoreI2C;
static SemaphoreHandle_t xSemaphoreDisplay;

// Variáveis de Controle e Limites (Atributos Compartilhados)
static bool g_display_on = true;
static float g_limiteVelocidade = 25.0;
static float g_limiteTemperatura = 70.0;
static bool g_alarme_ativo = false;

// Estruturas de dados
typedef struct {
    float temperature;
    float pressure;
    float altitude;
    TickType_t timestamp;
} SensorData_t;

typedef struct {
    float speedKmh;
    float distanceKm;
    TickType_t timestamp;
} SpeedData_t;

// --- Protótipos ---
void MqttTask(void *pvParameters);
void SensorTask(void *pvParameters);
void SpeedTask(void *pvParameters);
void DisplayTask(void *pvParameters);
void AlarmTask(void *pvParameters);

// --- Funções de Driver (Simuladas) ---
void i2c_master_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = PIN_I2C_SDA,
        .scl_io_num = PIN_I2C_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
}

void bmp280_read_data(float *temp, float *pressure, float *altitude) {
    // Simulação: Temperatura oscilando entre 20 e 80 para testar o alarme
    static float base_temp = 25.0;
    base_temp += 1.0; if (base_temp > 80.0) base_temp = 25.0; 
    *temp = base_temp;
    *pressure = 1013.25;
    *altitude = 100.0;
}

// --- MQTT Event Handler (RPC e Atributos) ---
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Conectado");
            // Inscreve para receber RPCs e atualizações de atributos
            esp_mqtt_client_subscribe(mqtt_client, "v1/devices/me/rpc/request/+", 0);
            esp_mqtt_client_subscribe(mqtt_client, "v1/devices/me/attributes", 0);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "Dados Recebidos: %.*s", event->data_len, event->data);
            
            // 1. Lógica de RPC (Switch Control)
            if (strstr(event->data, "set_display")) {
                g_display_on = strstr(event->data, "true") ? true : false;
                ESP_LOGI(TAG, "RPC: Display %s", g_display_on ? "LIGADO" : "DESLIGADO");
            }
            
            // 2. Lógica de Atributos Compartilhados (Limites)
            if (strstr(event->data, "limiteVelocidade")) {
                // Simplificação: Em produção usaria um parser JSON (ex: cJSON)
                // Aqui apenas logamos a recepção para fins didáticos
                ESP_LOGI(TAG, "Atributo Recebido: Novo limite de velocidade configurado");
            }
            if (strstr(event->data, "limiteTemperatura")) {
                ESP_LOGI(TAG, "Atributo Recebido: Novo limite de temperatura configurado");
            }
            break;
        default: break;
    }
}

// --- Task de Alarme (Buzzer) ---
void AlarmTask(void *pvParameters) {
    gpio_set_direction(PIN_BUZZER, GPIO_MODE_OUTPUT);
    SensorData_t sensor;
    SpeedData_t speed;

    for (;;) {
        xQueuePeek(xQueueSensorData, &sensor, 0);
        xQueuePeek(xQueueSpeedData, &speed, 0);

        if (speed.speedKmh > g_limiteVelocidade || sensor.temperature > g_limiteTemperatura) {
            g_alarme_ativo = true;
            gpio_set_level(PIN_BUZZER, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_set_level(PIN_BUZZER, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
            ESP_LOGW(TAG, "ALERTA! Limite ultrapassado. Temp: %.1f, Vel: %.1f", sensor.temperature, speed.speedKmh);
        } else {
            g_alarme_ativo = false;
            gpio_set_level(PIN_BUZZER, 0);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}

void MqttTask(void *pvParameters) {
    SensorData_t sensor;
    SpeedData_t speed;
    char json_payload[256];

    for (;;) {
        if (xQueuePeek(xQueueSensorData, &sensor, pdMS_TO_TICKS(1000)) == pdTRUE &&
            xQueuePeek(xQueueSpeedData, &speed, pdMS_TO_TICKS(1000)) == pdTRUE) {
            
            snprintf(json_payload, sizeof(json_payload),
                     "{\"velocidade\": %.2f, \"temperatura\": %.2f, \"distancia\": %.2f, \"altitude\": %.2f, \"bateria\": %d, \"alarme\": %s}",
                     speed.speedKmh, sensor.temperature, speed.distanceKm, sensor.altitude, 85, g_alarme_ativo ? "true" : "false");

            esp_mqtt_client_publish(mqtt_client, "v1/devices/me/telemetry", json_payload, 0, 1, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// --- Outras Tasks (Simplificadas) ---
void SpeedTask(void *pvParameters) {
    SpeedData_t speedData = {0};
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        speedData.speedKmh = 22.0; // Fixo para teste
        speedData.distanceKm += 0.01;
        xQueueOverwrite(xQueueSpeedData, &speedData);
    }
}

void SensorTask(void *pvParameters) {
    SensorData_t sensorData = {0};
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        bmp280_read_data(&sensorData.temperature, &sensorData.pressure, &sensorData.altitude);
        xQueueOverwrite(xQueueSensorData, &sensorData);
    }
}

void DisplayTask(void *pvParameters) {
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        if (g_display_on) {
            ESP_LOGI(TAG, "Display OLED Atualizado");
        }
    }
}

void app_main(void) {
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    example_connect();

    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URL,
        .credentials.username = MQTT_ACCESS_TOKEN,
    };
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);

    xQueueSensorData = xQueueCreate(1, sizeof(SensorData_t));
    xQueueSpeedData = xQueueCreate(1, sizeof(SpeedData_t));
    xSemaphoreI2C = xSemaphoreCreateMutex();
    xSemaphoreDisplay = xSemaphoreCreateMutex();

    xTaskCreate(SpeedTask,   "SpeedTask",   4096, NULL, 5, NULL);
    xTaskCreate(SensorTask,  "SensorTask",  4096, NULL, 4, NULL);
    xTaskCreate(DisplayTask, "DisplayTask", 4096, NULL, 3, NULL);
    xTaskCreate(MqttTask,    "MqttTask",    4096, NULL, 5, NULL);
    xTaskCreate(AlarmTask,   "AlarmTask",   2048, NULL, 6, NULL);
}
