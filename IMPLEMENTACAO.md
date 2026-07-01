# Guia de Implementação: Computador de Bordo para Ciclistas com ESP32

## 1. Lista de Componentes Necessários

### 1.1 Componentes Eletrônicos Principais

A construção do protótipo requer componentes de baixo custo, facilmente adquiríveis em plataformas de comércio eletrônico internacionais. A seleção foi realizada priorizando compatibilidade com o ESP32 e disponibilidade na lista de sensores do enunciado.

| Componente | Modelo/Especificação | Quantidade | Preço Estimado (USD) | Fornecedor | Observações |
|---|---|---|---|---|---|
| Microcontrolador | ESP32 DevKit v1 | 1 | 8-12 | AliExpress, Amazon | Inclui USB para programação |
| Sensor de Pressão/Temperatura | BMP280 (I2C) | 1 | 3-5 | AliExpress | Módulo pré-montado com breakout board |
| Acelerômetro/Giroscópio | MPU-6050 (I2C) | 1 | 4-6 | AliExpress | Sensor inercial de 6 eixos |
| Display OLED | SSD1306 0.96" (I2C) | 1 | 4-6 | AliExpress | Tela monocromática 128x64 pixels |
| Chave Magnética | Reed-Switch (2 unidades) | 2 | 1-2 | AliExpress | Para sensor de roda e cadência |
| Ímãs | Neodímio 5x5mm | 4 | 1-2 | AliExpress | Para acionamento dos Reed-Switches |
| Buzzer | Buzzer Ativo 5V | 1 | 1-2 | AliExpress | Para alertas sonoros |
| LEDs RGB | LED RGB 5mm com resistor | 2 | 1-2 | AliExpress | Para indicadores visuais |
| Botões | Botão Tátil 6x6mm | 3 | 0.5-1 | AliExpress | Para controle do dispositivo |
| Resistores | 10kΩ, 4.7kΩ, 220Ω | Variado | 1-2 | AliExpress | Para pull-up e limitação de corrente |
| Capacitores | 100µF, 10µF, 0.1µF | Variado | 1-2 | AliExpress | Para filtragem e desacoplamento |
| Bateria | Li-Po 3.7V 2000mAh | 1 | 8-12 | AliExpress | Com conector JST |
| Carregador | Carregador Li-Po USB | 1 | 5-8 | AliExpress | Com proteção de sobrecarga |
| Protoboard | Protoboard 400 furos | 1 | 2-3 | AliExpress | Para prototipagem |
| Fios de Conexão | Jumper wires (M-M, M-F) | 1 pacote | 2-3 | AliExpress | Variados comprimentos |
| Cabo USB | Micro-USB para programação | 1 | 2-3 | AliExpress | Para upload de código |

**Custo Total Estimado: USD 50-80 (aproximadamente R$ 250-400)**

### 1.2 Componentes Mecânicos e de Encapsulamento

Para transformar o protótipo em um dispositivo prático e durável, é necessário encapsulamento adequado e fixação na bicicleta.

| Item | Especificação | Quantidade | Preço Estimado | Observações |
|---|---|---|---|---|
| Case 3D Impresso | Caixa de proteção customizada | 1 | 15-30 | Impressão 3D em PLA ou PETG |
| Suporte de Guidão | Adaptador para fixação no guidão | 1 | 5-10 | Impressão 3D ou comprado pronto |
| Vedação | Silicone ou borracha impermeável | 1 | 5-10 | Para proteção contra chuva |
| Cabos de Fixação | Abraçadeiras de nylon | 1 pacote | 2-3 | Para fixação segura |
| Película Protetora | Película anti-reflexo para display | 1 | 3-5 | Opcional, melhora visibilidade |

**Custo Total Mecânico: USD 30-60**

---

## 2. Esquema de Conexões (Pinagem)

### 2.1 Diagrama de Conexões do ESP32

O ESP32 possui 38 pinos disponíveis, dos quais utilizaremos aproximadamente 15 para este projeto. A seleção de pinos foi realizada considerando as limitações de cada periférico.

```
┌─────────────────────────────────────────────────────────┐
│                    ESP32 DevKit v1                      │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  GND ─────────────────────────────────────────── GND    │
│  3V3 ─────────────────────────────────────────── 3V3    │
│                                                         │
│  GPIO 21 ────────────── I2C SDA (BMP280, MPU6050, OLED) │
│  GPIO 22 ────────────── I2C SCL (BMP280, MPU6050, OLED) │
│                                                         │
│  GPIO 25 ────────────── Reed-Switch Roda (Velocidade)   │
│  GPIO 26 ────────────── Reed-Switch Pedal (Cadência)    │
│                                                         │
│  GPIO 13 ────────────── Buzzer Ativo                    │
│  GPIO 14 ────────────── LED RGB R (Vermelho)           │
│  GPIO 27 ────────────── LED RGB G (Verde)              │
│  GPIO 12 ────────────── LED RGB B (Azul)               │
│                                                         │
│  GPIO 32 ────────────── Botão 1 (Iniciar/Parar)        │
│  GPIO 33 ────────────── Botão 2 (Lap/Volta)            │
│  GPIO 35 ────────────── Botão 3 (Menu)                 │
│                                                         │
│  GPIO 16 ────────────── UART RX (Futuro GPS)           │
│  GPIO 17 ────────────── UART TX (Futuro GPS)           │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

### 2.2 Tabela Detalhada de Conexões

| Componente | Pino ESP32 | Pino Componente | Protocolo | Observações |
|---|---|---|---|---|
| **BMP280** | GPIO 21 | SDA | I2C | Endereço I2C: 0x76 |
| **BMP280** | GPIO 22 | SCL | I2C | Velocidade: 100kHz |
| **BMP280** | 3V3 | VCC | Alimentação | 3.3V |
| **BMP280** | GND | GND | Alimentação | Terra comum |
| **MPU-6050** | GPIO 21 | SDA | I2C | Endereço I2C: 0x68 |
| **MPU-6050** | GPIO 22 | SCL | I2C | Velocidade: 100kHz |
| **MPU-6050** | 3V3 | VCC | Alimentação | 3.3V |
| **MPU-6050** | GND | GND | Alimentação | Terra comum |
| **OLED SSD1306** | GPIO 21 | SDA | I2C | Endereço I2C: 0x3C |
| **OLED SSD1306** | GPIO 22 | SCL | I2C | Velocidade: 100kHz |
| **OLED SSD1306** | 3V3 | VCC | Alimentação | 3.3V |
| **OLED SSD1306** | GND | GND | Alimentação | Terra comum |
| **Reed-Switch (Roda)** | GPIO 25 | Pino 1 | Digital | Com resistor pull-up 10kΩ |
| **Reed-Switch (Roda)** | GND | Pino 2 | Digital | Terra comum |
| **Reed-Switch (Pedal)** | GPIO 26 | Pino 1 | Digital | Com resistor pull-up 10kΩ |
| **Reed-Switch (Pedal)** | GND | Pino 2 | Digital | Terra comum |
| **Buzzer** | GPIO 13 | Positivo | Digital | PWM possível |
| **Buzzer** | GND | Negativo | Digital | Terra comum |
| **LED RGB R** | GPIO 14 | Ânodo | Digital | Com resistor 220Ω |
| **LED RGB G** | GPIO 27 | Ânodo | Digital | Com resistor 220Ω |
| **LED RGB B** | GPIO 12 | Ânodo | Digital | Com resistor 220Ω |
| **LED RGB** | GND | Cátodo Comum | Digital | Terra comum |
| **Botão 1** | GPIO 32 | Pino 1 | Digital | Com resistor pull-up 10kΩ |
| **Botão 1** | GND | Pino 2 | Digital | Terra comum |
| **Botão 2** | GPIO 33 | Pino 1 | Digital | Com resistor pull-up 10kΩ |
| **Botão 2** | GND | Pino 2 | Digital | Terra comum |
| **Botão 3** | GPIO 35 | Pino 1 | Digital | Com resistor pull-up 10kΩ |
| **Botão 3** | GND | Pino 2 | Digital | Terra comum |
| **Bateria Li-Po** | Vin | Positivo | Alimentação | Com proteção de sobrecarga |
| **Bateria Li-Po** | GND | Negativo | Alimentação | Terra comum |

---

## 3. Arquitetura de Software

### 3.1 Estrutura Geral do Projeto

O firmware do computador de bordo será organizado em módulos independentes, seguindo o padrão de programação embarcada profissional, permitindo reutilização de código e facilidade de manutenção.

```
BikeComputer_ESP32/
├── src/
│   ├── main.cpp                    # Arquivo principal com setup() e loop()
│   ├── sensors/
│   │   ├── bmp280_sensor.h        # Interface para sensor BMP280
│   │   ├── bmp280_sensor.cpp      # Implementação BMP280
│   │   ├── mpu6050_sensor.h       # Interface para sensor MPU-6050
│   │   ├── mpu6050_sensor.cpp     # Implementação MPU-6050
│   │   ├── speed_sensor.h         # Interface para sensor de velocidade
│   │   └── speed_sensor.cpp       # Implementação sensor de velocidade
│   ├── display/
│   │   ├── oled_display.h         # Interface para display OLED
│   │   └── oled_display.cpp       # Implementação OLED
│   ├── ui/
│   │   ├── ui_manager.h           # Gerenciador de interface
│   │   ├── ui_manager.cpp         # Implementação UI
│   │   ├── screens.h              # Definições de telas
│   │   └── screens.cpp            # Lógica de telas
│   ├── data/
│   │   ├── data_logger.h          # Interface para logging de dados
│   │   ├── data_logger.cpp        # Implementação data logger
│   │   └── trip_data.h            # Estrutura de dados de viagem
│   ├── control/
│   │   ├── button_handler.h       # Interface para botões
│   │   ├── button_handler.cpp     # Implementação botões
│   │   ├── led_controller.h       # Interface para LEDs
│   │   └── led_controller.cpp     # Implementação LEDs
│   ├── power/
│   │   ├── power_manager.h        # Gerenciador de energia
│   │   └── power_manager.cpp      # Implementação power manager
│   └── config.h                   # Configurações globais
├── lib/
│   ├── Adafruit_BMP280/           # Biblioteca BMP280
│   ├── MPU6050/                   # Biblioteca MPU-6050
│   ├── Adafruit_SSD1306/          # Biblioteca OLED
│   └── Adafruit_GFX/              # Biblioteca gráfica
├── platformio.ini                 # Configuração PlatformIO
└── README.md                      # Documentação do projeto
```

### 3.2 Fluxo de Execução Principal

O firmware segue um padrão de máquina de estados para gerenciar os diferentes modos de operação do dispositivo.

```
┌─────────────────────────────────────────────────────────┐
│                    INICIALIZAÇÃO                        │
│  • Configurar pinos GPIO                                │
│  • Inicializar I2C e sensores                           │
│  • Carregar dados persistentes                          │
│  • Exibir tela de boas-vindas                           │
└──────────────────┬──────────────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────────────┐
│                 LOOP PRINCIPAL (10ms)                   │
├─────────────────────────────────────────────────────────┤
│  1. Verificar entrada de botões                         │
│  2. Ler sensores (BMP280, MPU-6050)                     │
│  3. Processar pulsos de velocidade/cadência             │
│  4. Calcular métricas (velocidade, elevação)            │
│  5. Atualizar display OLED                              │
│  6. Registrar dados em memória flash                    │
│  7. Gerenciar estado de energia                         │
└──────────────────┬──────────────────────────────────────┘
                   │
        ┌──────────┼──────────┐
        ▼          ▼          ▼
    ┌────────┐ ┌────────┐ ┌────────┐
    │ PARADO │ │ ATIVO  │ │ PAUSA  │
    └────────┘ └────────┘ └────────┘
        │          │          │
        └──────────┼──────────┘
                   │
                   ▼
    ┌──────────────────────────────────┐
    │ Verificar condições de saída     │
    │ • Bateria baixa                  │
    │ • Memória cheia                  │
    │ • Comando do usuário             │
    └──────────────────────────────────┘
```

---

## 4. Código-Fonte Detalhado

### 4.1 Arquivo de Configuração (config.h)

O arquivo de configuração centraliza todas as constantes e parâmetros do sistema, facilitando ajustes sem necessidade de recompilação completa.

```cpp
// config.h - Configurações Globais do Sistema

#ifndef CONFIG_H
#define CONFIG_H

// ============ CONFIGURAÇÕES DE HARDWARE ============

// Pinos GPIO
#define PIN_I2C_SDA             21
#define PIN_I2C_SCL             22
#define PIN_SPEED_SENSOR        25      // Reed-Switch roda
#define PIN_CADENCE_SENSOR      26      // Reed-Switch pedal
#define PIN_BUZZER              13
#define PIN_LED_R               14
#define PIN_LED_G               27
#define PIN_LED_B               12
#define PIN_BUTTON_START        32      // Iniciar/Parar
#define PIN_BUTTON_LAP          33      // Marcar volta
#define PIN_BUTTON_MENU         35      // Menu

// Endereços I2C
#define I2C_ADDR_BMP280         0x76    // Sensor de pressão
#define I2C_ADDR_MPU6050        0x68    // Sensor inercial
#define I2C_ADDR_OLED           0x3C    // Display OLED

// ============ CONFIGURAÇÕES DE SENSORES ============

// BMP280
#define BMP280_SAMPLING_RATE    1000    // ms entre leituras
#define BMP280_SEA_LEVEL_PA     101325  // Pressão ao nível do mar

// MPU-6050
#define MPU6050_SAMPLING_RATE   100     // ms entre leituras
#define MPU6050_ACCEL_RANGE     16      // ±16g

// Sensor de Velocidade
#define WHEEL_CIRCUMFERENCE     2.1     // metros (pneu 700x25c)
#define DEBOUNCE_TIME           50      // ms para debouncing

// Sensor de Cadência
#define CADENCE_WINDOW          1000    // ms para cálculo de RPM

// ============ CONFIGURAÇÕES DE DISPLAY ============

#define SCREEN_WIDTH            128
#define SCREEN_HEIGHT           64
#define SCREEN_UPDATE_RATE      500     // ms entre atualizações

// Estados de tela
#define SCREEN_HOME             0
#define SCREEN_STATS            1
#define SCREEN_ALTITUDE         2
#define SCREEN_SETTINGS         3

// ============ CONFIGURAÇÕES DE BATERIA ============

#define BATTERY_LOW_THRESHOLD   3.2     // Volts
#define BATTERY_CRITICAL        3.0     // Volts
#define ADC_PIN                 34      // Pino ADC para leitura de bateria

// ============ CONFIGURAÇÕES DE ARMAZENAMENTO ============

#define LOG_INTERVAL            5000    // ms entre registros de dados
#define MAX_TRIPS               50      // Máximo de viagens armazenadas
#define SPIFFS_MOUNT_POINT      "/spiffs"

// ============ CONFIGURAÇÕES DE TEMPO ============

#define MAIN_LOOP_RATE          10      // ms (100 Hz)
#define SENSOR_READ_RATE        100     // ms

// ============ LIMITES E THRESHOLDS ============

#define MAX_SPEED               100     // km/h
#define MAX_ALTITUDE            9000    // metros
#define IMPACT_THRESHOLD        2.5     // g (detecção de queda)

#endif // CONFIG_H
```

### 4.2 Arquivo Principal (main.cpp)

O arquivo principal contém a lógica de inicialização e o loop principal do sistema.

```cpp
// main.cpp - Arquivo Principal

#include <Arduino.h>
#include <Wire.h>
#include <SPIFFS.h>
#include "config.h"
#include "sensors/bmp280_sensor.h"
#include "sensors/mpu6050_sensor.h"
#include "sensors/speed_sensor.h"
#include "display/oled_display.h"
#include "control/button_handler.h"
#include "control/led_controller.h"
#include "data/data_logger.h"
#include "power/power_manager.h"

// Instâncias globais dos módulos
BMP280Sensor bmp280;
MPU6050Sensor mpu6050;
SpeedSensor speedSensor;
OLEDDisplay display;
ButtonHandler buttons;
LEDController leds;
DataLogger dataLogger;
PowerManager powerManager;

// Variáveis de estado
enum SystemState {
    STATE_IDLE,
    STATE_RECORDING,
    STATE_PAUSED,
    STATE_MENU
};

SystemState currentState = STATE_IDLE;
unsigned long lastSensorRead = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long tripStartTime = 0;

// Estrutura de dados da viagem atual
struct TripData {
    float totalDistance;        // km
    float maxSpeed;             // km/h
    float avgSpeed;             // km/h
    float maxAltitude;          // m
    float minAltitude;          // m
    float totalElevation;       // m
    unsigned long tripDuration; // ms
    float avgCadence;           // RPM
    int impactCount;            // Número de impactos detectados
} currentTrip;

// ============ FUNÇÕES DE INICIALIZAÇÃO ============

void initializeSensors() {
    Serial.println("[INIT] Inicializando sensores...");
    
    // Inicializar I2C
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(100000);  // 100 kHz
    
    // Inicializar BMP280
    if (!bmp280.begin(I2C_ADDR_BMP280)) {
        Serial.println("[ERROR] BMP280 não encontrado!");
    } else {
        Serial.println("[OK] BMP280 inicializado");
    }
    
    // Inicializar MPU-6050
    if (!mpu6050.begin(I2C_ADDR_MPU6050)) {
        Serial.println("[ERROR] MPU-6050 não encontrado!");
    } else {
        Serial.println("[OK] MPU-6050 inicializado");
    }
    
    // Inicializar sensor de velocidade
    speedSensor.begin(PIN_SPEED_SENSOR, WHEEL_CIRCUMFERENCE);
    
    Serial.println("[INIT] Sensores inicializados com sucesso");
}

void initializePeripherals() {
    Serial.println("[INIT] Inicializando periféricos...");
    
    // Inicializar display OLED
    if (!display.begin(I2C_ADDR_OLED, SCREEN_WIDTH, SCREEN_HEIGHT)) {
        Serial.println("[ERROR] Display OLED não encontrado!");
    } else {
        Serial.println("[OK] Display OLED inicializado");
        display.displayWelcomeScreen();
    }
    
    // Inicializar botões
    buttons.begin(PIN_BUTTON_START, PIN_BUTTON_LAP, PIN_BUTTON_MENU);
    
    // Inicializar LEDs
    leds.begin(PIN_LED_R, PIN_LED_G, PIN_LED_B);
    leds.setColor(0, 255, 0);  // Verde inicial
    
    // Inicializar buzzer
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);
    
    Serial.println("[INIT] Periféricos inicializados com sucesso");
}

void initializeStorage() {
    Serial.println("[INIT] Inicializando armazenamento...");
    
    if (!SPIFFS.begin(true)) {
        Serial.println("[ERROR] Falha ao montar SPIFFS!");
        return;
    }
    
    // Inicializar data logger
    if (!dataLogger.begin(SPIFFS_MOUNT_POINT)) {
        Serial.println("[ERROR] Falha ao inicializar data logger!");
    } else {
        Serial.println("[OK] Data logger inicializado");
    }
    
    Serial.println("[INIT] Armazenamento inicializado com sucesso");
}

void initializePower() {
    Serial.println("[INIT] Inicializando gerenciador de energia...");
    
    if (!powerManager.begin(ADC_PIN)) {
        Serial.println("[ERROR] Falha ao inicializar power manager!");
    } else {
        Serial.println("[OK] Power manager inicializado");
    }
    
    Serial.println("[INIT] Gerenciador de energia inicializado");
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n========== BIKE COMPUTER ESP32 ==========");
    Serial.println("Versão 1.0");
    Serial.println("Iniciando sistema...\n");
    
    // Sequência de inicialização
    initializeSensors();
    delay(500);
    
    initializePeripherals();
    delay(500);
    
    initializeStorage();
    delay(500);
    
    initializePower();
    delay(500);
    
    // Resetar dados da viagem
    resetTripData();
    
    Serial.println("\n[INIT] Sistema pronto para operação!");
    Serial.println("=========================================\n");
    
    // Exibir tela inicial
    display.displayHomeScreen();
}

// ============ FUNÇÕES AUXILIARES ============

void resetTripData() {
    currentTrip.totalDistance = 0.0;
    currentTrip.maxSpeed = 0.0;
    currentTrip.avgSpeed = 0.0;
    currentTrip.maxAltitude = 0.0;
    currentTrip.minAltitude = 9999.0;
    currentTrip.totalElevation = 0.0;
    currentTrip.tripDuration = 0;
    currentTrip.avgCadence = 0.0;
    currentTrip.impactCount = 0;
}

void handleStateTransition(SystemState newState) {
    if (newState == currentState) return;
    
    Serial.printf("[STATE] Transição: %d -> %d\n", currentState, newState);
    
    switch (newState) {
        case STATE_RECORDING:
            tripStartTime = millis();
            leds.setColor(255, 0, 0);  // Vermelho
            buzzerBeep(2, 100);
            Serial.println("[STATE] Iniciando gravação");
            break;
            
        case STATE_PAUSED:
            leds.setColor(255, 255, 0);  // Amarelo
            buzzerBeep(1, 150);
            Serial.println("[STATE] Viagem pausada");
            break;
            
        case STATE_IDLE:
            leds.setColor(0, 255, 0);  // Verde
            buzzerBeep(1, 100);
            Serial.println("[STATE] Sistema ocioso");
            break;
            
        case STATE_MENU:
            leds.setColor(0, 0, 255);  // Azul
            Serial.println("[STATE] Menu aberto");
            break;
    }
    
    currentState = newState;
}

void buzzerBeep(int count, int duration) {
    for (int i = 0; i < count; i++) {
        digitalWrite(PIN_BUZZER, HIGH);
        delay(duration);
        digitalWrite(PIN_BUZZER, LOW);
        delay(duration);
    }
}

// ============ LOOP PRINCIPAL ============

void loop() {
    unsigned long currentTime = millis();
    
    // Leitura de sensores (100 Hz)
    if (currentTime - lastSensorRead >= SENSOR_READ_RATE) {
        lastSensorRead = currentTime;
        readSensors();
    }
    
    // Atualização de display (2 Hz)
    if (currentTime - lastDisplayUpdate >= SCREEN_UPDATE_RATE) {
        lastDisplayUpdate = currentTime;
        updateDisplay();
    }
    
    // Processamento de botões
    handleButtonInput();
    
    // Verificação de bateria
    checkBatteryStatus();
    
    // Logging de dados (a cada 5 segundos)
    if (currentState == STATE_RECORDING && 
        currentTime % LOG_INTERVAL < MAIN_LOOP_RATE) {
        logTripData();
    }
    
    // Pequeno delay para evitar travamento
    delay(MAIN_LOOP_RATE);
}

void readSensors() {
    // Ler BMP280
    float temperature, pressure, altitude;
    if (bmp280.readData(temperature, pressure, altitude)) {
        // Atualizar altitude máxima e mínima
        if (altitude > currentTrip.maxAltitude) {
            currentTrip.maxAltitude = altitude;
        }
        if (altitude < currentTrip.minAltitude) {
            currentTrip.minAltitude = altitude;
        }
    }
    
    // Ler MPU-6050
    float accelX, accelY, accelZ;
    if (mpu6050.readAccel(accelX, accelY, accelZ)) {
        // Calcular magnitude de aceleração
        float accelMagnitude = sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ);
        
        // Detectar impacto (queda)
        if (accelMagnitude > IMPACT_THRESHOLD && currentState == STATE_RECORDING) {
            currentTrip.impactCount++;
            leds.setColor(255, 0, 0);  // Piscar vermelho
            buzzerBeep(3, 50);
            Serial.printf("[ALERT] Impacto detectado! Magnitude: %.2f g\n", accelMagnitude);
        }
    }
    
    // Ler sensor de velocidade
    float speed = speedSensor.getSpeed();
    if (speed > currentTrip.maxSpeed) {
        currentTrip.maxSpeed = speed;
    }
}

void updateDisplay() {
    switch (currentState) {
        case STATE_IDLE:
            display.displayHomeScreen();
            break;
            
        case STATE_RECORDING:
            display.displayRecordingScreen(
                currentTrip.totalDistance,
                speedSensor.getSpeed(),
                currentTrip.maxSpeed,
                currentTrip.avgSpeed
            );
            break;
            
        case STATE_PAUSED:
            display.displayPausedScreen(
                currentTrip.tripDuration / 1000,
                currentTrip.totalDistance
            );
            break;
            
        case STATE_MENU:
            display.displayMenuScreen();
            break;
    }
}

void handleButtonInput() {
    ButtonEvent event = buttons.getEvent();
    
    switch (event) {
        case BUTTON_START_PRESSED:
            if (currentState == STATE_IDLE) {
                handleStateTransition(STATE_RECORDING);
            } else if (currentState == STATE_RECORDING) {
                handleStateTransition(STATE_PAUSED);
            } else if (currentState == STATE_PAUSED) {
                handleStateTransition(STATE_RECORDING);
            }
            break;
            
        case BUTTON_LAP_PRESSED:
            if (currentState == STATE_RECORDING) {
                // Marcar volta
                dataLogger.logLap(currentTrip.totalDistance, 
                                 millis() - tripStartTime);
                buzzerBeep(1, 200);
                Serial.println("[LAP] Volta marcada");
            }
            break;
            
        case BUTTON_MENU_PRESSED:
            if (currentState != STATE_MENU) {
                handleStateTransition(STATE_MENU);
            } else {
                handleStateTransition(STATE_IDLE);
            }
            break;
            
        default:
            break;
    }
}

void checkBatteryStatus() {
    float voltage = powerManager.getBatteryVoltage();
    
    if (voltage < BATTERY_CRITICAL) {
        // Bateria crítica - desligar
        Serial.println("[CRITICAL] Bateria crítica! Desligando...");
        leds.setColor(255, 0, 0);
        buzzerBeep(5, 100);
        delay(1000);
        // Aqui seria implementado deep sleep
    } else if (voltage < BATTERY_LOW_THRESHOLD) {
        // Bateria baixa - avisar
        leds.setColor(255, 165, 0);  // Laranja
        Serial.printf("[WARNING] Bateria baixa: %.2f V\n", voltage);
    }
}

void logTripData() {
    // Calcular tempo decorrido
    currentTrip.tripDuration = millis() - tripStartTime;
    
    // Calcular velocidade média
    if (currentTrip.tripDuration > 0) {
        currentTrip.avgSpeed = (currentTrip.totalDistance * 3600000.0) / 
                               currentTrip.tripDuration;
    }
    
    // Registrar dados
    dataLogger.logData(
        currentTrip.totalDistance,
        speedSensor.getSpeed(),
        currentTrip.maxSpeed,
        currentTrip.avgSpeed,
        bmp280.getAltitude(),
        currentTrip.totalElevation,
        mpu6050.getTemperature()
    );
}
```

### 4.3 Módulo de Sensor BMP280 (bmp280_sensor.h e .cpp)

```cpp
// bmp280_sensor.h

#ifndef BMP280_SENSOR_H
#define BMP280_SENSOR_H

#include <Adafruit_BMP280.h>

class BMP280Sensor {
private:
    Adafruit_BMP280 bmp;
    float lastAltitude;
    float baseAltitude;
    float totalElevation;
    
public:
    BMP280Sensor();
    bool begin(uint8_t address);
    bool readData(float &temperature, float &pressure, float &altitude);
    float getAltitude();
    float getTemperature();
    float getPressure();
    float getTotalElevation();
};

#endif // BMP280_SENSOR_H
```

```cpp
// bmp280_sensor.cpp

#include "bmp280_sensor.h"

BMP280Sensor::BMP280Sensor() : lastAltitude(0), baseAltitude(0), totalElevation(0) {}

bool BMP280Sensor::begin(uint8_t address) {
    if (!bmp.begin(address)) {
        return false;
    }
    
    // Configurar sensor para melhor precisão
    bmp.setSampling(
        Adafruit_BMP280::MODE_NORMAL,
        Adafruit_BMP280::SAMPLING_X2,
        Adafruit_BMP280::SAMPLING_X16,
        Adafruit_BMP280::FILTER_X16,
        Adafruit_BMP280::STANDBY_MS_500
    );
    
    baseAltitude = bmp.readAltitude(101325);
    lastAltitude = baseAltitude;
    
    return true;
}

bool BMP280Sensor::readData(float &temperature, float &pressure, float &altitude) {
    temperature = bmp.readTemperature();
    pressure = bmp.readPressure();
    altitude = bmp.readAltitude(101325);
    
    // Calcular elevação total
    if (altitude > lastAltitude) {
        totalElevation += (altitude - lastAltitude);
    }
    lastAltitude = altitude;
    
    return true;
}

float BMP280Sensor::getAltitude() {
    return bmp.readAltitude(101325);
}

float BMP280Sensor::getTemperature() {
    return bmp.readTemperature();
}

float BMP280Sensor::getPressure() {
    return bmp.readPressure();
}

float BMP280Sensor::getTotalElevation() {
    return totalElevation;
}
```

### 4.4 Módulo de Sensor de Velocidade (speed_sensor.h e .cpp)

```cpp
// speed_sensor.h

#ifndef SPEED_SENSOR_H
#define SPEED_SENSOR_H

#include <Arduino.h>

class SpeedSensor {
private:
    int sensorPin;
    float wheelCircumference;
    volatile unsigned long pulseCount;
    unsigned long lastPulseTime;
    float currentSpeed;
    float totalDistance;
    
    static void IRAM_ATTR pulseISR();
    static SpeedSensor* instance;
    
public:
    SpeedSensor();
    void begin(int pin, float circumference);
    float getSpeed();
    float getTotalDistance();
    void reset();
};

#endif // SPEED_SENSOR_H
```

```cpp
// speed_sensor.cpp

#include "speed_sensor.h"

SpeedSensor* SpeedSensor::instance = nullptr;

SpeedSensor::SpeedSensor() : sensorPin(-1), wheelCircumference(2.1), 
                             pulseCount(0), lastPulseTime(0), 
                             currentSpeed(0), totalDistance(0) {
    instance = this;
}

void IRAM_ATTR SpeedSensor::pulseISR() {
    if (instance != nullptr) {
        instance->pulseCount++;
    }
}

void SpeedSensor::begin(int pin, float circumference) {
    sensorPin = pin;
    wheelCircumference = circumference;
    
    pinMode(sensorPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(sensorPin), pulseISR, FALLING);
}

float SpeedSensor::getSpeed() {
    unsigned long currentTime = millis();
    unsigned long timeDelta = currentTime - lastPulseTime;
    
    if (timeDelta > 1000) {
        // Se passou mais de 1 segundo sem pulso, velocidade é zero
        currentSpeed = 0;
    } else if (pulseCount > 0) {
        // Calcular velocidade em km/h
        // Distância = pulsos * circunferência da roda
        // Velocidade = distância / tempo
        float distancePerSecond = (pulseCount * wheelCircumference) / 
                                  (timeDelta / 1000.0);
        currentSpeed = distancePerSecond * 3.6;  // Converter m/s para km/h
        
        totalDistance += (pulseCount * wheelCircumference) / 1000.0;  // em km
        pulseCount = 0;
    }
    
    lastPulseTime = currentTime;
    return currentSpeed;
}

float SpeedSensor::getTotalDistance() {
    return totalDistance;
}

void SpeedSensor::reset() {
    pulseCount = 0;
    currentSpeed = 0;
    totalDistance = 0;
    lastPulseTime = millis();
}
```

### 4.5 Módulo de Display OLED (oled_display.h e .cpp)

```cpp
// oled_display.h

#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

class OLEDDisplay {
private:
    Adafruit_SSD1306 display;
    
public:
    OLEDDisplay();
    bool begin(uint8_t address, int width, int height);
    void displayWelcomeScreen();
    void displayHomeScreen();
    void displayRecordingScreen(float distance, float speed, 
                               float maxSpeed, float avgSpeed);
    void displayPausedScreen(unsigned long elapsedSeconds, float distance);
    void displayMenuScreen();
    void displayAltitudeScreen(float altitude, float totalElevation);
};

#endif // OLED_DISPLAY_H
```

```cpp
// oled_display.cpp

#include "oled_display.h"

OLEDDisplay::OLEDDisplay() : display(128, 64, &Wire, -1) {}

bool OLEDDisplay::begin(uint8_t address, int width, int height) {
    if (!display.begin(SSD1306_SWITCHCAPVCC, address)) {
        return false;
    }
    
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.clearDisplay();
    display.display();
    
    return true;
}

void OLEDDisplay::displayWelcomeScreen() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(20, 10);
    display.println("BIKE");
    display.println("COMPUTER");
    
    display.setTextSize(1);
    display.setCursor(30, 50);
    display.println("v1.0 - ESP32");
    
    display.display();
}

void OLEDDisplay::displayHomeScreen() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("=== PRONTO ===");
    display.println();
    display.println("Pressione START");
    display.println("para iniciar");
    display.println();
    display.println("Distancia: 0.0 km");
    display.println("Velocidade: 0 km/h");
    
    display.display();
}

void OLEDDisplay::displayRecordingScreen(float distance, float speed, 
                                        float maxSpeed, float avgSpeed) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("=== GRAVANDO ===");
    
    display.setTextSize(2);
    display.setCursor(0, 15);
    display.printf("%.1f km\n", distance);
    
    display.setTextSize(1);
    display.printf("Vel: %.1f km/h\n", speed);
    display.printf("Max: %.1f km/h\n", maxSpeed);
    display.printf("Med: %.1f km/h\n", avgSpeed);
    
    display.display();
}

void OLEDDisplay::displayPausedScreen(unsigned long elapsedSeconds, float distance) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("=== PAUSADO ===");
    
    unsigned long hours = elapsedSeconds / 3600;
    unsigned long minutes = (elapsedSeconds % 3600) / 60;
    unsigned long seconds = elapsedSeconds % 60;
    
    display.setTextSize(2);
    display.setCursor(0, 15);
    display.printf("%02ld:%02ld:%02ld\n", hours, minutes, seconds);
    
    display.setTextSize(1);
    display.printf("Distancia: %.1f km\n", distance);
    
    display.display();
}

void OLEDDisplay::displayMenuScreen() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("=== MENU ===");
    display.println();
    display.println("1. Historico");
    display.println("2. Configuracoes");
    display.println("3. Sobre");
    display.println();
    display.println("Pressione MENU");
    display.println("para sair");
    
    display.display();
}

void OLEDDisplay::displayAltitudeScreen(float altitude, float totalElevation) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("=== ALTITUDE ===");
    
    display.setTextSize(2);
    display.setCursor(0, 15);
    display.printf("%.0f m\n", altitude);
    
    display.setTextSize(1);
    display.printf("Subida: %.0f m\n", totalElevation);
    
    display.display();
}
```

### 4.6 Módulo de Controle de Botões (button_handler.h e .cpp)

```cpp
// button_handler.h

#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>

enum ButtonEvent {
    BUTTON_NONE,
    BUTTON_START_PRESSED,
    BUTTON_LAP_PRESSED,
    BUTTON_MENU_PRESSED
};

class ButtonHandler {
private:
    int pinStart, pinLap, pinMenu;
    unsigned long lastDebounceStart, lastDebounceLap, lastDebounceMenu;
    bool lastStateStart, lastStateLap, lastStateMenu;
    const unsigned long DEBOUNCE_DELAY = 50;  // ms
    
public:
    ButtonHandler();
    void begin(int pinStart, int pinLap, int pinMenu);
    ButtonEvent getEvent();
    
private:
    bool readButton(int pin, unsigned long &lastDebounce, bool &lastState);
};

#endif // BUTTON_HANDLER_H
```

```cpp
// button_handler.cpp

#include "button_handler.h"

ButtonHandler::ButtonHandler() : pinStart(-1), pinLap(-1), pinMenu(-1),
                                lastDebounceStart(0), lastDebounceLap(0), 
                                lastDebounceMenu(0),
                                lastStateStart(HIGH), lastStateLap(HIGH), 
                                lastStateMenu(HIGH) {}

void ButtonHandler::begin(int pinStart, int pinLap, int pinMenu) {
    this->pinStart = pinStart;
    this->pinLap = pinLap;
    this->pinMenu = pinMenu;
    
    pinMode(pinStart, INPUT_PULLUP);
    pinMode(pinLap, INPUT_PULLUP);
    pinMode(pinMenu, INPUT_PULLUP);
}

ButtonEvent ButtonHandler::getEvent() {
    if (readButton(pinStart, lastDebounceStart, lastStateStart)) {
        return BUTTON_START_PRESSED;
    }
    
    if (readButton(pinLap, lastDebounceLap, lastStateLap)) {
        return BUTTON_LAP_PRESSED;
    }
    
    if (readButton(pinMenu, lastDebounceMenu, lastStateMenu)) {
        return BUTTON_MENU_PRESSED;
    }
    
    return BUTTON_NONE;
}

bool ButtonHandler::readButton(int pin, unsigned long &lastDebounce, bool &lastState) {
    bool currentState = digitalRead(pin);
    unsigned long currentTime = millis();
    
    if (currentTime - lastDebounce > DEBOUNCE_DELAY) {
        if (currentState != lastState) {
            lastState = currentState;
            lastDebounce = currentTime;
            
            // Retornar true apenas quando o botão é pressionado (LOW)
            if (currentState == LOW) {
                return true;
            }
        }
    }
    
    return false;
}
```

---

## 5. Instruções de Instalação e Configuração

### 5.1 Ambiente de Desenvolvimento

O projeto utiliza PlatformIO como gerenciador de dependências e compilador, oferecendo melhor suporte para ESP32 comparado ao Arduino IDE tradicional.

**Passo 1: Instalar PlatformIO**

```bash
# Instalar via pip
pip install platformio

# Ou via instalador gráfico
# https://platformio.org/install/ide
```

**Passo 2: Criar Projeto**

```bash
# Criar novo projeto
platformio project init --board esp32doit-devkit-v1 --framework arduino

# Ou clonar este repositório
git clone https://github.com/seu-usuario/BikeComputer_ESP32.git
cd BikeComputer_ESP32
```

**Passo 3: Instalar Dependências**

```bash
# Adicionar bibliotecas necessárias
platformio lib install "Adafruit BMP280"
platformio lib install "MPU6050"
platformio lib install "Adafruit SSD1306"
platformio lib install "Adafruit GFX Library"
```

### 5.2 Arquivo platformio.ini

```ini
[env:esp32doit-devkit-v1]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino
monitor_speed = 115200
upload_speed = 921600

lib_deps =
    adafruit/Adafruit BMP280 Library@^2.6.8
    jrowberg/MPU6050@^0.12.0
    adafruit/Adafruit SSD1306@^2.5.7
    adafruit/Adafruit GFX Library@^1.11.5

build_flags =
    -DCORE_DEBUG_LEVEL=2
```

### 5.3 Compilação e Upload

```bash
# Compilar código
platformio run

# Upload para ESP32
platformio run --target upload

# Monitorar saída serial
platformio device monitor --baud 115200
```

---

## 6. Testes e Validação

### 6.1 Testes Unitários

Cada módulo deve ser testado independentemente antes da integração.

```cpp
// test_bmp280.cpp - Teste do sensor BMP280

#include <Arduino.h>
#include "sensors/bmp280_sensor.h"

void test_bmp280() {
    BMP280Sensor bmp280;
    
    if (!bmp280.begin(0x76)) {
        Serial.println("FAIL: BMP280 não inicializou");
        return;
    }
    
    Serial.println("PASS: BMP280 inicializou com sucesso");
    
    // Testar leitura
    float temp, pressure, altitude;
    if (bmp280.readData(temp, pressure, altitude)) {
        Serial.printf("Temperatura: %.2f C\n", temp);
        Serial.printf("Pressão: %.2f Pa\n", pressure);
        Serial.printf("Altitude: %.2f m\n", altitude);
        Serial.println("PASS: Leitura de dados bem-sucedida");
    } else {
        Serial.println("FAIL: Falha ao ler dados");
    }
}
```

### 6.2 Teste de Integração

Teste completo do sistema em operação.

```cpp
// test_integration.cpp - Teste de integração

void test_full_system() {
    Serial.println("\n=== TESTE DE INTEGRAÇÃO ===\n");
    
    // 1. Teste de sensores
    Serial.println("1. Testando sensores...");
    test_bmp280();
    test_mpu6050();
    test_speed_sensor();
    
    // 2. Teste de display
    Serial.println("\n2. Testando display...");
    display.displayWelcomeScreen();
    delay(2000);
    display.displayHomeScreen();
    
    // 3. Teste de botões
    Serial.println("\n3. Testando botões...");
    Serial.println("Pressione os botões...");
    for (int i = 0; i < 10; i++) {
        ButtonEvent event = buttons.getEvent();
        if (event != BUTTON_NONE) {
            Serial.printf("Botão pressionado: %d\n", event);
        }
        delay(100);
    }
    
    // 4. Teste de LEDs
    Serial.println("\n4. Testando LEDs...");
    leds.setColor(255, 0, 0);    // Vermelho
    delay(500);
    leds.setColor(0, 255, 0);    // Verde
    delay(500);
    leds.setColor(0, 0, 255);    // Azul
    delay(500);
    leds.setColor(0, 0, 0);      // Desligado
    
    Serial.println("\n=== TESTES CONCLUÍDOS ===\n");
}
```

---

## 7. Otimizações de Performance e Energia

### 7.1 Gerenciamento de Energia

Para maximizar a duração da bateria, implementar técnicas de power management.

```cpp
// power_manager.cpp - Estratégias de economia de energia

void PowerManager::optimizeForBattery() {
    // Reduzir frequência do clock quando inativo
    setCpuFrequencyMhz(80);  // Reduzir de 240 MHz para 80 MHz
    
    // Desligar Wi-Fi quando não necessário
    WiFi.mode(WIFI_OFF);
    
    // Usar deep sleep quando possível
    if (shouldEnterDeepSleep()) {
        esp_sleep_enable_timer_wakeup(60 * 1000000);  // Acordar em 60 segundos
        esp_deep_sleep_start();
    }
}
```

### 7.2 Otimização de Leitura de Sensores

Implementar buffering e processamento em lote para reduzir overhead.

```cpp
// sensor_buffer.h - Buffer de sensores

class SensorBuffer {
private:
    static const int BUFFER_SIZE = 10;
    float buffer[BUFFER_SIZE];
    int index;
    
public:
    void addReading(float value) {
        buffer[index] = value;
        index = (index + 1) % BUFFER_SIZE;
    }
    
    float getAverage() {
        float sum = 0;
        for (int i = 0; i < BUFFER_SIZE; i++) {
            sum += buffer[i];
        }
        return sum / BUFFER_SIZE;
    }
};
```

---

## 8. Troubleshooting e Debugging

### 8.1 Problemas Comuns

| Problema | Causa Provável | Solução |
|---|---|---|
| ESP32 não conecta via USB | Driver USB não instalado | Instalar driver CH340 ou CP2102 |
| Sensores não respondem | Conexão I2C incorreta | Verificar pinos SDA/SCL e endereços I2C |
| Display não exibe nada | Endereço I2C errado | Usar scanner I2C para encontrar endereço correto |
| Bateria drena rapidamente | Wi-Fi sempre ativo | Implementar duty cycling do Wi-Fi |
| Leitura de velocidade instável | Debouncing inadequado | Aumentar DEBOUNCE_TIME em config.h |

### 8.2 Ferramentas de Debug

```cpp
// debug.h - Macros de debug

#ifdef DEBUG_ENABLED
    #define DEBUG_PRINT(x) Serial.print(x)
    #define DEBUG_PRINTLN(x) Serial.println(x)
    #define DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, __VA_ARGS__)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTF(fmt, ...)
#endif
```

---

## Referências

[1] Espressif Systems. "ESP32 Technical Reference Manual." Disponível em: https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf

[2] Adafruit. "BMP280 Barometric Pressure & Altitude Sensor." Disponível em: https://learn.adafruit.com/adafruit-bmp280-barometric-pressure-plus-altitude-sensor-breakout

[3] InvenSense. "MPU-6050 Product Specification." Disponível em: https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000_DataSheet_en.pdf

[4] Adafruit. "Monochrome OLED Breakouts." Disponível em: https://learn.adafruit.com/monochrome-oled-breakouts

[5] Arduino. "Arduino Language Reference." Disponível em: https://www.arduino.cc/reference/en/

[6] PlatformIO. "PlatformIO Documentation." Disponível em: https://docs.platformio.org/
