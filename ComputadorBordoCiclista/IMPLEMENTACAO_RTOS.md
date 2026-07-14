# Implementação com FreeRTOS — Computador de Bordo para Ciclistas

O ESP32 executa o FreeRTOS nativamente como parte do framework ESP-IDF e do Arduino Core. A adoção de um RTOS transforma a arquitetura de software de um loop sequencial único para um modelo de tarefas concorrentes com prioridades definidas, comunicação via filas e sincronização via semáforos. Este documento descreve como a implementação do computador de bordo se beneficia dessa abordagem.

---

## 1. Por que usar FreeRTOS neste projeto

No modelo sem RTOS (loop sequencial), todas as operações ocorrem em série dentro do `loop()` do Arduino. Isso cria um problema estrutural: a leitura do sensor BMP280 via I2C pode levar até 40 ms, o que atrasa a atualização do display, que por sua vez atrasa a leitura dos botões. Em um ciclocomputador, a perda de um pulso do Reed-Switch durante esse intervalo representa um erro direto no cálculo de velocidade.

Com FreeRTOS, cada responsabilidade do sistema é isolada em uma **task** independente com sua própria pilha de execução e prioridade. O escalonador do FreeRTOS garante que tarefas críticas (como a leitura de interrupções de hardware) sejam atendidas dentro de prazos determinísticos, enquanto tarefas de menor prioridade (como gravação em flash) aguardam nos momentos de ociosidade do processador.

O ESP32 possui dois núcleos (Core 0 e Core 1), o que permite distribuir as tarefas fisicamente entre os núcleos, eliminando contenção de CPU entre operações de I/O e processamento de dados.

---

## 2. Arquitetura de Tarefas

### 2.1 Diagrama de Tarefas e Comunicação

```
┌─────────────────────────────────────────────────────────────────────┐
│                         CORE 0 (Protocolo)                          │
├──────────────────────────┬──────────────────────────────────────────┤
│   Task: SensorTask       │   Task: DataLoggerTask                   │
│   Prioridade: 4          │   Prioridade: 1                          │
│   Stack: 4096 bytes      │   Stack: 4096 bytes                      │
│   Período: 100 ms        │   Período: 5000 ms                       │
│   • Lê BMP280 (I2C)      │   • Grava dados em SPIFFS                │
│   • Lê MPU-6050 (I2C)    │   • Serializa struct TripData            │
│   • Publica em Queue     │   • Aguarda SensorQueue                  │
└──────────────────────────┴──────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│                         CORE 1 (Aplicação)                          │
├──────────────────────────┬──────────────────────────────────────────┤
│   Task: DisplayTask      │   Task: ButtonTask                       │
│   Prioridade: 3          │   Prioridade: 5 (mais alta)              │
│   Stack: 8192 bytes      │   Stack: 2048 bytes                      │
│   Período: 200 ms        │   Período: 20 ms (debounce)              │
│   • Lê SensorQueue       │   • Lê GPIO 32, 33, 35                   │
│   • Renderiza OLED       │   • Envia evento para EventQueue         │
│   • Gerencia telas       │   • Detecta pressionamento longo         │
├──────────────────────────┴──────────────────────────────────────────┤
│   Task: SpeedTask                                                    │
│   Prioridade: 6 (crítica — acionada por interrupção)                │
│   Stack: 2048 bytes                                                  │
│   • Aguarda notificação de ISR (Reed-Switch)                        │
│   • Calcula velocidade e cadência                                    │
│   • Publica em SpeedQueue                                           │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│                    PRIMITIVAS DE SINCRONIZAÇÃO                      │
├──────────────────────────────────────────────────────────────────────┤
│  xQueueSensorData    → SensorTask  ──→  DisplayTask, DataLoggerTask │
│  xQueueSpeedData     → SpeedTask   ──→  DisplayTask                 │
│  xQueueButtonEvent   → ButtonTask  ──→  DisplayTask                 │
│  xSemaphoreI2C       → Mutex para acesso exclusivo ao barramento I2C│
│  xSemaphoreDisplay   → Mutex para acesso exclusivo ao OLED          │
└─────────────────────────────────────────────────────────────────────┘
```

### 2.2 Tabela de Tarefas

| Task | Núcleo | Prioridade | Período | Stack | Responsabilidade |
|---|---|---|---|---|---|
| `SpeedTask` | Core 1 | 6 | Orientada a evento (ISR) | 2048 B | Cálculo de velocidade e cadência via Reed-Switch |
| `ButtonTask` | Core 1 | 5 | 20 ms | 2048 B | Leitura e debounce de botões físicos |
| `SensorTask` | Core 0 | 4 | 100 ms | 4096 B | Leitura de BMP280 e MPU-6050 via I2C |
| `DisplayTask` | Core 1 | 3 | 200 ms | 8192 B | Renderização do display OLED e lógica de telas |
| `AlarmTask` | Core 0 | 4 | 50 ms | 2048 B | Monitoramento de limiares e acionamento de buzzer/LED |
| `DataLoggerTask` | Core 0 | 1 | 5000 ms | 4096 B | Gravação periódica de dados em memória SPIFFS |

> A prioridade 6 da `SpeedTask` garante que nenhuma outra tarefa interrompa o processamento de um pulso do Reed-Switch, evitando erros de contagem de rotações.

---

## 3. Estruturas de Dados Compartilhadas

```cpp
// Dados publicados pela SensorTask
typedef struct {
    float temperature;      // °C — leitura BMP280
    float pressure;         // hPa — leitura BMP280
    float altitude;         // metros — calculado a partir da pressão
    float accelX;           // g — MPU-6050 eixo X
    float accelY;           // g — MPU-6050 eixo Y
    float accelZ;           // g — MPU-6050 eixo Z
    float gyroX;            // °/s — MPU-6050 eixo X
    float gyroY;            // °/s — MPU-6050 eixo Y
    float gyroZ;            // °/s — MPU-6050 eixo Z
    float gradient;         // % — inclinação calculada
    TickType_t timestamp;   // tick do FreeRTOS no momento da leitura
} SensorData_t;

// Dados publicados pela SpeedTask
typedef struct {
    float speedKmh;         // km/h — velocidade instantânea
    float cadenceRpm;       // RPM — cadência de pedalada
    float distanceKm;       // km — distância acumulada
    uint32_t wheelPulses;   // contagem total de pulsos da roda
    TickType_t timestamp;
} SpeedData_t;

// Eventos publicados pela ButtonTask
typedef enum {
    BTN_START_STOP,         // Botão 1: iniciar ou parar atividade
    BTN_LAP,                // Botão 2: marcar volta (lap)
    BTN_MENU,               // Botão 3: abrir menu / trocar tela
    BTN_MENU_LONG           // Botão 3 pressionado por > 2s: configurações
} ButtonEvent_t;

// Handles globais das filas e semáforos
QueueHandle_t   xQueueSensorData;
QueueHandle_t   xQueueSpeedData;
QueueHandle_t   xQueueButtonEvent;
SemaphoreHandle_t xSemaphoreI2C;
SemaphoreHandle_t xSemaphoreDisplay;
```

---

## 4. Implementação das Tarefas

### 4.1 Inicialização e criação das tarefas (`main.cpp`)

```cpp
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "config.h"

// Declaração das funções de tarefa
void SpeedTask(void *pvParameters);
void ButtonTask(void *pvParameters);
void SensorTask(void *pvParameters);
void DisplayTask(void *pvParameters);
void AlarmTask(void *pvParameters);
void DataLoggerTask(void *pvParameters);

// ISR para Reed-Switch
TaskHandle_t xSpeedTaskHandle = NULL;

void IRAM_ATTR wheelISR() {
    // Notifica a SpeedTask a partir da ISR sem bloquear
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(xSpeedTaskHandle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void IRAM_ATTR cadenceISR() {
    // Notificação separada para cadência (implementação análoga)
}

void setup() {
    Serial.begin(115200);

    // Criar filas
    xQueueSensorData  = xQueueCreate(5, sizeof(SensorData_t));
    xQueueSpeedData   = xQueueCreate(10, sizeof(SpeedData_t));
    xQueueButtonEvent = xQueueCreate(5, sizeof(ButtonEvent_t));

    // Criar semáforos mutex
    xSemaphoreI2C     = xSemaphoreCreateMutex();
    xSemaphoreDisplay = xSemaphoreCreateMutex();

    // Configurar interrupções dos Reed-Switches
    pinMode(PIN_REED_WHEEL, INPUT_PULLUP);
    pinMode(PIN_REED_CADENCE, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_REED_WHEEL), wheelISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_REED_CADENCE), cadenceISR, FALLING);

    // Criar tarefas — parâmetros: função, nome, stack, param, prioridade, handle, núcleo
    xTaskCreatePinnedToCore(SpeedTask,      "SpeedTask",      2048, NULL, 6, &xSpeedTaskHandle, 1);
    xTaskCreatePinnedToCore(ButtonTask,     "ButtonTask",     2048, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(SensorTask,     "SensorTask",     4096, NULL, 4, NULL, 0);
    xTaskCreatePinnedToCore(AlarmTask,      "AlarmTask",      2048, NULL, 4, NULL, 0);
    xTaskCreatePinnedToCore(DisplayTask,    "DisplayTask",    8192, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(DataLoggerTask, "DataLoggerTask", 4096, NULL, 1, NULL, 0);

    // O loop() do Arduino não é utilizado — as tarefas gerenciam tudo
}

void loop() {
    // Intencionalmente vazio — o FreeRTOS assume o controle
    vTaskDelay(portMAX_DELAY);
}
```

### 4.2 SpeedTask — Cálculo de velocidade por interrupção

```cpp
void SpeedTask(void *pvParameters) {
    SpeedData_t speedData = {0};
    TickType_t lastWheelTick = 0;
    uint32_t totalPulses = 0;

    const float WHEEL_CIRCUMFERENCE_M = 2.10f;  // 700x25c em metros

    for (;;) {
        // Aguarda notificação da ISR do Reed-Switch (bloqueante, sem polling)
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        TickType_t now = xTaskGetTickCount();
        TickType_t interval = now - lastWheelTick;
        lastWheelTick = now;
        totalPulses++;

        // Calcular velocidade: v = circunferência / tempo_entre_pulsos
        if (interval > 0 && interval < pdMS_TO_TICKS(5000)) {
            float intervalSec = (float)interval / configTICK_RATE_HZ;
            speedData.speedKmh   = (WHEEL_CIRCUMFERENCE_M / intervalSec) * 3.6f;
            speedData.distanceKm = (totalPulses * WHEEL_CIRCUMFERENCE_M) / 1000.0f;
        } else {
            // Sem pulso por mais de 5 segundos: velocidade zero
            speedData.speedKmh = 0.0f;
        }

        speedData.wheelPulses = totalPulses;
        speedData.timestamp   = now;

        // Publicar na fila sem bloquear (descarta se fila cheia)
        xQueueOverwrite(xQueueSpeedData, &speedData);
    }
}
```

### 4.3 SensorTask — Leitura periódica de BMP280 e MPU-6050

```cpp
void SensorTask(void *pvParameters) {
    SensorData_t sensorData = {0};
    float altitudePrev = 0.0f;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(100);  // 10 Hz

    // Inicializar sensores I2C (protegido por mutex)
    if (xSemaphoreTake(xSemaphoreI2C, pdMS_TO_TICKS(1000)) == pdTRUE) {
        bmp280.begin(0x76);
        mpu6050.begin();
        xSemaphoreGive(xSemaphoreI2C);
    }

    for (;;) {
        // Aguardar até o próximo período (execução periódica precisa)
        vTaskDelayUntil(&xLastWakeTime, xPeriod);

        // Adquirir mutex do barramento I2C antes de qualquer leitura
        if (xSemaphoreTake(xSemaphoreI2C, pdMS_TO_TICKS(50)) == pdTRUE) {

            // Leitura BMP280
            sensorData.temperature = bmp280.readTemperature();
            sensorData.pressure    = bmp280.readPressure() / 100.0f;  // Pa → hPa
            sensorData.altitude    = bmp280.readAltitude(1013.25f);

            // Leitura MPU-6050
            mpu6050.getAcceleration(
                &sensorData.accelX, &sensorData.accelY, &sensorData.accelZ
            );
            mpu6050.getRotation(
                &sensorData.gyroX, &sensorData.gyroY, &sensorData.gyroZ
            );

            xSemaphoreGive(xSemaphoreI2C);
        }

        // Calcular inclinação a partir da variação de altitude e distância
        // gradient (%) = (Δaltitude / distância_horizontal) × 100
        sensorData.gradient  = calculateGradient(sensorData.altitude, altitudePrev);
        altitudePrev         = sensorData.altitude;
        sensorData.timestamp = xTaskGetTickCount();

        // Publicar na fila (sobrescreve o dado mais antigo se fila cheia)
        xQueueSend(xQueueSensorData, &sensorData, 0);
    }
}
```

### 4.4 DisplayTask — Renderização do OLED

```cpp
void DisplayTask(void *pvParameters) {
    SensorData_t  sensorData  = {0};
    SpeedData_t   speedData   = {0};
    ButtonEvent_t btnEvent;
    uint8_t       currentScreen = SCREEN_MAIN;
    TickType_t    xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod   = pdMS_TO_TICKS(200);  // 5 Hz

    // Inicializar display (protegido por mutex)
    if (xSemaphoreTake(xSemaphoreDisplay, pdMS_TO_TICKS(1000)) == pdTRUE) {
        display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
        display.clearDisplay();
        display.display();
        xSemaphoreGive(xSemaphoreDisplay);
    }

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);

        // Processar eventos de botão (não bloqueante)
        if (xQueueReceive(xQueueButtonEvent, &btnEvent, 0) == pdTRUE) {
            handleButtonEvent(btnEvent, &currentScreen);
        }

        // Obter dados mais recentes das filas (não bloqueante)
        xQueuePeek(xQueueSensorData, &sensorData, 0);
        xQueuePeek(xQueueSpeedData,  &speedData,  0);

        // Renderizar tela atual com mutex do display
        if (xSemaphoreTake(xSemaphoreDisplay, pdMS_TO_TICKS(50)) == pdTRUE) {
            display.clearDisplay();

            switch (currentScreen) {
                case SCREEN_MAIN:
                    renderMainScreen(&sensorData, &speedData);
                    break;
                case SCREEN_ALTITUDE:
                    renderAltitudeScreen(&sensorData);
                    break;
                case SCREEN_CADENCE:
                    renderCadenceScreen(&speedData);
                    break;
                case SCREEN_STATS:
                    renderStatsScreen(&sensorData, &speedData);
                    break;
            }

            display.display();
            xSemaphoreGive(xSemaphoreDisplay);
        }
    }
}
```

### 4.5 ButtonTask — Leitura com debounce

```cpp
void ButtonTask(void *pvParameters) {
    const TickType_t DEBOUNCE_MS    = pdMS_TO_TICKS(20);
    const TickType_t LONG_PRESS_MS  = pdMS_TO_TICKS(2000);

    bool btn1Prev = HIGH, btn2Prev = HIGH, btn3Prev = HIGH;
    TickType_t btn3PressTime = 0;

    for (;;) {
        vTaskDelay(DEBOUNCE_MS);

        bool btn1 = digitalRead(PIN_BTN1);
        bool btn2 = digitalRead(PIN_BTN2);
        bool btn3 = digitalRead(PIN_BTN3);
        ButtonEvent_t event;

        // Botão 1: borda de descida → iniciar/parar
        if (btn1 == LOW && btn1Prev == HIGH) {
            event = BTN_START_STOP;
            xQueueSend(xQueueButtonEvent, &event, 0);
        }

        // Botão 2: borda de descida → lap
        if (btn2 == LOW && btn2Prev == HIGH) {
            event = BTN_LAP;
            xQueueSend(xQueueButtonEvent, &event, 0);
        }

        // Botão 3: pressionamento curto → menu; longo → configurações
        if (btn3 == LOW && btn3Prev == HIGH) {
            btn3PressTime = xTaskGetTickCount();
        }
        if (btn3 == HIGH && btn3Prev == LOW) {
            TickType_t duration = xTaskGetTickCount() - btn3PressTime;
            event = (duration >= LONG_PRESS_MS) ? BTN_MENU_LONG : BTN_MENU;
            xQueueSend(xQueueButtonEvent, &event, 0);
        }

        btn1Prev = btn1;
        btn2Prev = btn2;
        btn3Prev = btn3;
    }
}
```

### 4.6 AlarmTask — Monitoramento de limiares

```cpp
void AlarmTask(void *pvParameters) {
    SensorData_t sensorData = {0};
    SpeedData_t  speedData  = {0};
    TickType_t   xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod   = pdMS_TO_TICKS(50);  // 20 Hz

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);

        xQueuePeek(xQueueSensorData, &sensorData, 0);
        xQueuePeek(xQueueSpeedData,  &speedData,  0);

        // Verificar limiar de inclinação (gradiente > 12% = alarme crítico)
        if (sensorData.gradient > GRADIENT_EXTREME) {
            triggerAlarm(ALARM_CRITICAL);
        } else if (sensorData.gradient > GRADIENT_STEEP) {
            triggerAlarm(ALARM_WARNING);
        }

        // Verificar detecção de queda (aceleração > 2.5g + giroscópio > 300°/s)
        float accelMag = sqrt(
            sensorData.accelX * sensorData.accelX +
            sensorData.accelY * sensorData.accelY +
            sensorData.accelZ * sensorData.accelZ
        );
        float gyroMag = sqrt(
            sensorData.gyroX * sensorData.gyroX +
            sensorData.gyroY * sensorData.gyroY +
            sensorData.gyroZ * sensorData.gyroZ
        );

        if (accelMag > IMPACT_THRESHOLD && gyroMag > GYRO_THRESHOLD) {
            triggerAlarm(ALARM_CRASH);
        }

        // Verificar temperatura extrema
        if (sensorData.temperature > TEMP_HIGH_THRESHOLD) {
            triggerAlarm(ALARM_TEMPERATURE);
        }
    }
}
```

### 4.7 DataLoggerTask — Gravação periódica em SPIFFS

```cpp
void DataLoggerTask(void *pvParameters) {
    SensorData_t sensorData = {0};
    SpeedData_t  speedData  = {0};
    TickType_t   xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod   = pdMS_TO_TICKS(5000);  // A cada 5 segundos

    // Inicializar SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("[DataLogger] Falha ao montar SPIFFS");
        vTaskDelete(NULL);
    }

    File logFile = SPIFFS.open("/trip_log.csv", FILE_APPEND);
    if (!logFile) {
        Serial.println("[DataLogger] Falha ao abrir arquivo de log");
        vTaskDelete(NULL);
    }
    logFile.println("timestamp,speed,distance,altitude,temperature,gradient");
    logFile.close();

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);

        // Obter dados mais recentes
        xQueuePeek(xQueueSensorData, &sensorData, 0);
        xQueuePeek(xQueueSpeedData,  &speedData,  0);

        // Gravar linha CSV no arquivo
        File f = SPIFFS.open("/trip_log.csv", FILE_APPEND);
        if (f) {
            f.printf("%lu,%.2f,%.3f,%.1f,%.1f,%.1f\n",
                (unsigned long)xTaskGetTickCount(),
                speedData.speedKmh,
                speedData.distanceKm,
                sensorData.altitude,
                sensorData.temperature,
                sensorData.gradient
            );
            f.close();
        }
    }
}
```

---

## 5. Diagrama de Estados com RTOS

```
                    ┌──────────────────────────────────────────────┐
                    │              ESTADO DO SISTEMA               │
                    └──────────────────────────────────────────────┘

  ┌──────────┐  BTN_START  ┌──────────┐  BTN_START  ┌──────────────┐
  │  PARADO  │────────────▶│  ATIVO   │────────────▶│   PAUSADO    │
  │          │             │          │             │              │
  │ Display  │             │ Todas as │             │ SpeedTask    │
  │ mostra   │             │ tasks    │             │ suspensa     │
  │ resumo   │◀────────────│ ativas   │◀────────────│ Display      │
  └──────────┘  BTN_STOP   └────┬─────┘  BTN_START  │ congelado    │
                                │                   └──────────────┘
                                │ ALARM_CRASH
                                ▼
                    ┌──────────────────────┐
                    │   EMERGÊNCIA (SOS)   │
                    │ Buzzer contínuo      │
                    │ LED vermelho piscando│
                    │ Display: "QUEDA!"    │
                    │ Aguarda confirmação  │
                    └──────────────────────┘
```

---

## 6. Gerenciamento de Energia com RTOS

Com FreeRTOS, o gerenciamento de energia é integrado ao escalonador. Quando todas as tarefas estão bloqueadas aguardando filas ou delays, o ESP32 entra automaticamente em modo de baixo consumo (Automatic Light Sleep).

```cpp
// Configurar Light Sleep automático no ESP32
// O FreeRTOS entra em sleep quando o idle task é executado
esp_pm_config_esp32_t pm_config = {
    .max_freq_mhz       = 240,   // Frequência máxima durante processamento
    .min_freq_mhz       = 80,    // Frequência mínima durante idle
    .light_sleep_enable = true   // Habilitar sleep automático
};
esp_pm_configure(&pm_config);
```

A tabela abaixo mostra o consumo estimado em cada estado do sistema com o gerenciamento de energia ativo:

| Estado do Sistema | Tasks Ativas | Frequência CPU | Consumo Estimado |
|---|---|---|---|
| **Idle (sem movimento)** | Apenas ButtonTask aguardando | 80 MHz | ~15 mA |
| **Ciclismo ativo** | Todas as tasks | 160 MHz | ~110 mA |
| **Tela desligada** | SpeedTask + SensorTask | 80 MHz | ~45 mA |
| **Deep Sleep** | Nenhuma (apenas RTC) | — | ~0.01 mA |

---

## 7. Comparativo: Loop Sequencial vs. FreeRTOS

| Critério | Loop Sequencial (`loop()`) | FreeRTOS (`xTaskCreate`) |
|---|---|---|
| **Latência de botão** | Até 200 ms (bloqueado por I2C) | < 20 ms (task dedicada) |
| **Perda de pulsos Reed-Switch** | Possível durante leitura I2C | Impossível (ISR + notificação) |
| **Uso dos dois núcleos** | Apenas Core 1 | Core 0 e Core 1 distribuídos |
| **Isolamento de falhas** | Falha em um sensor trava tudo | Watchdog por task independente |
| **Consumo de energia** | Sem gerenciamento automático | Light Sleep automático no idle |
| **Complexidade de código** | Baixa | Moderada |
| **Determinismo temporal** | Não garantido | Garantido por prioridades |

---

## 8. Configuração do `platformio.ini` para FreeRTOS

```ini
[env:esp32dev]
platform  = espressif32
board     = esp32dev
framework = arduino

; Bibliotecas necessárias
lib_deps =
    adafruit/Adafruit BMP280 Library @ ^2.6.8
    electroniccats/MPU6050 @ ^1.3.0
    adafruit/Adafruit SSD1306 @ ^2.5.9
    adafruit/Adafruit GFX Library @ ^1.11.9

; Configurações de compilação
build_flags =
    -DCONFIG_FREERTOS_HZ=1000
    -DCORE_DEBUG_LEVEL=3

; Monitor serial
monitor_speed = 115200
```

> A diretiva `CONFIG_FREERTOS_HZ=1000` configura o tick do FreeRTOS para 1 ms, permitindo delays com resolução de 1 ms via `vTaskDelay(pdMS_TO_TICKS(1))`.

---

## Referências

[1] Espressif Systems. "FreeRTOS — ESP-IDF Programming Guide." Disponível em: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/freertos.html

[2] FreeRTOS. "FreeRTOS Reference Manual." Disponível em: https://www.freertos.org/Documentation/RTOS_book.html

[3] Kolban, N. "Kolban's Book on ESP32." Disponível em: https://leanpub.com/kolban-ESP32

[4] Espressif Systems. "Power Management — ESP-IDF Programming Guide." Disponível em: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/power_management.html
