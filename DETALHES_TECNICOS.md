# Detalhes Técnicos Avançados: Computador de Bordo para Ciclistas

## 1. Especificações de Bateria e Gerenciamento de Energia

### 1.1 Seleção de Bateria para o Projeto

A escolha da bateria é crítica para o sucesso do projeto, influenciando diretamente a autonomia operacional e a viabilidade prática do dispositivo. Para este projeto com ESP32, foi selecionada uma bateria de **Lítio-Polímero (Li-Po) 3.7V 2000mAh**, que representa um equilíbrio ótimo entre capacidade, tamanho e custo.

| Parâmetro | Especificação | Unidade | Justificativa |
|---|---|---|---|
| **Tipo de Bateria** | Lítio-Polímero (Li-Po) | - | Melhor densidade energética comparada a NiMH ou alcalinas |
| **Tensão Nominal** | 3.7V | V | Compatível diretamente com reguladores de 3.3V do ESP32 |
| **Capacidade** | 2000 mAh | mAh | Oferece 7.4 Wh de energia total |
| **Dimensões** | 60 x 36 x 7 mm | mm | Cabe em encapsulamento compacto para guidão |
| **Peso** | 34 gramas | g | Adição mínima ao peso total da bicicleta |
| **Tensão de Carga Máxima** | 4.2V | V | Limite superior para evitar danos |
| **Tensão de Descarga Mínima** | 3.0V | V | Limite inferior para preservar ciclos de vida |
| **Taxa de Descarga Contínua** | 2C (4A) | A | Suficiente para picos de consumo do ESP32 |
| **Ciclos de Carga/Descarga** | 500-1000 | ciclos | Vida útil típica com uso adequado |

**Energia Total Disponível**: 2000 mAh × 3.7V = 7.4 Wh (ou 26,640 Joules)

### 1.2 Perfil de Consumo de Energia do Sistema

O consumo de energia varia significativamente dependendo do estado operacional do sistema. A análise detalhada dos diferentes modos permite otimizar a autonomia através de gerenciamento inteligente.

| Modo Operacional | Componentes Ativos | Consumo Típico | Consumo Pico | Duração Esperada |
|---|---|---|---|---|
| **Deep Sleep** | RTC, Watchdog | 10 µA | 20 µA | ~3000 horas (125 dias) |
| **Light Sleep** | CPU desligado, WiFi desligado | 0.8 mA | 1.5 mA | ~370 horas (15 dias) |
| **Idle (CPU 80 MHz)** | CPU em espera, sensores inativos | 40 mA | 60 mA | ~9 horas |
| **Leitura de Sensores** | I2C ativo, BMP280, MPU6050 | 60 mA | 80 mA | ~6 horas |
| **Display OLED Ativo** | OLED + CPU + Sensores | 120 mA | 150 mA | ~3 horas |
| **WiFi Ativo** | WiFi TX/RX, CPU 240 MHz | 160 mA | 500 mA | ~2 horas |
| **Operação Completa** | Todos os componentes | 140 mA | 200 mA | ~2.5 horas |

**Nota**: Os valores acima são estimativas baseadas em datasheets e devem ser validados através de testes práticos com o protótipo.

### 1.3 Cálculo de Autonomia Operacional

Para uma viagem típica de ciclismo com o sistema em operação completa (display ligado, sensores ativos, mas WiFi desligado):

```
Consumo Médio Estimado: 120 mA
Capacidade da Bateria: 2000 mAh
Autonomia = 2000 mAh ÷ 120 mA = 16.67 horas

Autonomia Prática: ~15-16 horas com display sempre ligado
Autonomia Otimizada: ~20-24 horas com display em modo econômico
```

Este cálculo assume que o WiFi permanece desligado durante a atividade. Ativar WiFi para sincronização reduz a autonomia para aproximadamente 2-3 horas.

### 1.4 Estratégias de Otimização de Energia

Para maximizar a duração da bateria durante uma atividade de ciclismo, implementar as seguintes estratégias de software:

#### 1.4.1 Duty Cycling Adaptativo

O duty cycling consiste em ligar e desligar componentes de forma inteligente baseado no estado do sistema.

```cpp
// Exemplo de duty cycling adaptativo para sensores
void adaptiveSensorSampling() {
    // Se velocidade > 20 km/h, ler sensores a cada 100ms
    if (currentSpeed > 20) {
        SENSOR_READ_INTERVAL = 100;  // Alta frequência
    }
    // Se velocidade entre 5-20 km/h, ler a cada 200ms
    else if (currentSpeed > 5) {
        SENSOR_READ_INTERVAL = 200;  // Frequência média
    }
    // Se velocidade < 5 km/h (parado), ler a cada 500ms
    else {
        SENSOR_READ_INTERVAL = 500;  // Baixa frequência
    }
}
```

#### 1.4.2 Redução Dinâmica de Frequência de Clock

O processador ESP32 pode operar em diferentes frequências, reduzindo o consumo quando não há processamento intensivo.

```cpp
// Reduzir frequência de clock em modo econômico
void setPowerMode(PowerMode mode) {
    switch (mode) {
        case POWER_PERFORMANCE:
            setCpuFrequencyMhz(240);  // Máxima performance, máximo consumo
            break;
        case POWER_BALANCED:
            setCpuFrequencyMhz(160);  // Balanço entre performance e consumo
            break;
        case POWER_ECONOMY:
            setCpuFrequencyMhz(80);   // Economia de energia, performance reduzida
            break;
    }
}
```

#### 1.4.3 Desligamento Seletivo de Componentes

Desligar componentes que não estão em uso reduz significativamente o consumo.

```cpp
// Desligar WiFi quando não necessário
void disableWiFiWhenIdle() {
    if (shouldSyncData()) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(SSID, PASSWORD);
        // Sincronizar dados
        syncDataToCloud();
        // Desligar WiFi após sincronização
        WiFi.mode(WIFI_OFF);
    }
}

// Reduzir brilho do display em ambientes escuros
void adjustDisplayBrightness() {
    int ambientLight = readLDRSensor();
    if (ambientLight < 100) {
        display.setBrightness(50);    // 50% de brilho
    } else if (ambientLight < 500) {
        display.setBrightness(100);   // 100% de brilho
    } else {
        display.setBrightness(200);   // 200% de brilho (máximo)
    }
}
```

### 1.5 Monitoramento de Bateria em Tempo Real

Implementar leitura contínua da tensão da bateria para alertar o usuário sobre bateria baixa.

```cpp
// Classe para monitoramento de bateria
class BatteryMonitor {
private:
    const int ADC_PIN = 34;
    const float VOLTAGE_DIVIDER = 2.0;  // Divisor de tensão
    const float ADC_RESOLUTION = 4095.0;
    const float REFERENCE_VOLTAGE = 3.3;
    
public:
    float getBatteryVoltage() {
        int adcValue = analogRead(ADC_PIN);
        float voltage = (adcValue / ADC_RESOLUTION) * REFERENCE_VOLTAGE * VOLTAGE_DIVIDER;
        return voltage;
    }
    
    int getBatteryPercentage() {
        float voltage = getBatteryVoltage();
        
        // Mapeamento linear de 3.0V (0%) a 4.2V (100%)
        if (voltage >= 4.2) return 100;
        if (voltage <= 3.0) return 0;
        
        return (int)((voltage - 3.0) / (4.2 - 3.0) * 100);
    }
    
    BatteryStatus getStatus() {
        float voltage = getBatteryVoltage();
        
        if (voltage >= 3.8) return BATTERY_GOOD;
        if (voltage >= 3.5) return BATTERY_WARNING;
        if (voltage >= 3.0) return BATTERY_CRITICAL;
        return BATTERY_DEAD;
    }
};
```

---

## 2. Especificações Detalhadas de Sensores

### 2.1 Sensor de Pressão e Temperatura BMP280

O BMP280 é um sensor barométrico de alta precisão que permite medir altitude e temperatura com excelente acurácia.

| Especificação | Valor | Unidade | Observações |
|---|---|---|---|
| **Intervalo de Temperatura** | -40 a +85 | °C | Operação em condições extremas |
| **Precisão de Temperatura** | ±1.0 | °C | Típica em condições normais |
| **Intervalo de Pressão** | 300 a 1100 | hPa | Equivalente a 0 a 9000m de altitude |
| **Precisão de Pressão** | ±1.0 | hPa | Resolução de ~8 metros de altitude |
| **Tempo de Resposta** | < 1 | segundo | Para mudanças de altitude |
| **Consumo de Corrente** | 2.7 | µA | Em modo sleep |
| **Consumo Operacional** | 0.6 | mA | Durante leitura ativa |
| **Protocolo de Comunicação** | I2C/SPI | - | Utilizamos I2C (100 kHz) |
| **Endereço I2C** | 0x76 ou 0x77 | - | Configurável por pino SDO |
| **Taxa de Amostragem** | Até 127 Hz | Hz | Configurável em firmware |

**Aplicação no Projeto**: O BMP280 fornece dados de elevação em tempo real, permitindo calcular a subida acumulada (climbing) e a inclinação atual da via. Estes dados são essenciais para análise de performance de ciclismo.

#### 2.1.1 Calibração do BMP280

O sensor BMP280 contém coeficientes de calibração armazenados internamente que devem ser lidos durante a inicialização.

```cpp
// Inicialização com calibração automática
bool BMP280Sensor::begin(uint8_t address) {
    if (!bmp.begin(address)) {
        Serial.println("BMP280 não encontrado!");
        return false;
    }
    
    // Configurar para melhor precisão
    bmp.setSampling(
        Adafruit_BMP280::MODE_NORMAL,        // Modo contínuo
        Adafruit_BMP280::SAMPLING_X16,       // Sobreamostragem de temperatura
        Adafruit_BMP280::SAMPLING_X16,       // Sobreamostragem de pressão
        Adafruit_BMP280::FILTER_X16,         // Filtro IIR
        Adafruit_BMP280::STANDBY_MS_500      // Intervalo de espera
    );
    
    // Ler altitude de referência (nível do mar)
    baseAltitude = bmp.readAltitude(101325);  // 101325 Pa = 1 atm
    
    return true;
}
```

### 2.2 Sensor Inercial MPU-6050 (Acelerômetro e Giroscópio)

O MPU-6050 é uma Unidade de Medição Inercial (IMU) de 6 eixos que combina acelerômetro e giroscópio em um único chip.

| Especificação | Valor | Unidade | Observações |
|---|---|---|---|
| **Faixa de Aceleração** | ±2, ±4, ±8, ±16 | g | Configurável, usamos ±16g |
| **Resolução do Acelerômetro** | 16 bits | - | Resolução de 0.488 mg/LSB em ±16g |
| **Faixa de Giroscópio** | ±250, ±500, ±1000, ±2000 | °/s | Configurável, usamos ±2000°/s |
| **Resolução do Giroscópio** | 16 bits | - | Resolução de 0.061 °/s/LSB em ±2000°/s |
| **Frequência de Amostragem** | Até 8000 | Hz | Configurável via registrador SMPRT_DIV |
| **Consumo de Corrente** | 3.7 | mA | Durante operação normal |
| **Consumo Sleep** | 5 | µA | Em modo sleep |
| **Protocolo** | I2C/SPI | - | Utilizamos I2C (400 kHz) |
| **Endereço I2C** | 0x68 ou 0x69 | - | Configurável por pino AD0 |
| **Temperatura Operacional** | -40 a +85 | °C | Faixa de operação |

**Aplicação no Projeto**: O MPU-6050 detecta movimentos e impactos, permitindo identificar quedas, saltos (em mountain bike) e calcular a inclinação da bicicleta. Os dados de giroscópio ajudam a determinar a orientação do dispositivo.

#### 2.2.1 Detecção de Incidentes (Quedas)

A detecção de incidentes utiliza a magnitude de aceleração combinada com análise de mudanças rápidas.

```cpp
// Algoritmo de detecção de queda
class FallDetectionAlgorithm {
private:
    const float IMPACT_THRESHOLD = 2.5;      // 2.5g de aceleração
    const float TILT_THRESHOLD = 45.0;       // 45 graus de inclinação
    const int WINDOW_SIZE = 5;               // Janela de 5 amostras
    float accelBuffer[WINDOW_SIZE];
    int bufferIndex = 0;
    
public:
    bool detectFall(float accelX, float accelY, float accelZ, 
                   float gyroX, float gyroY, float gyroZ) {
        
        // Calcular magnitude de aceleração
        float accelMagnitude = sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ);
        
        // Adicionar ao buffer
        accelBuffer[bufferIndex] = accelMagnitude;
        bufferIndex = (bufferIndex + 1) % WINDOW_SIZE;
        
        // Calcular média da janela
        float avgAccel = 0;
        for (int i = 0; i < WINDOW_SIZE; i++) {
            avgAccel += accelBuffer[i];
        }
        avgAccel /= WINDOW_SIZE;
        
        // Critério 1: Aceleração alta
        bool highAccel = avgAccel > IMPACT_THRESHOLD;
        
        // Critério 2: Mudança rápida de orientação (giroscópio)
        float gyroMagnitude = sqrt(gyroX*gyroX + gyroY*gyroY + gyroZ*gyroZ);
        bool rapidRotation = gyroMagnitude > 300;  // 300 °/s
        
        // Detectar queda se ambos os critérios forem atendidos
        return (highAccel && rapidRotation);
    }
};
```

**Limiares de Detecção Utilizados**:
- **Aceleração de Impacto**: 2.5g (25 m/s²) - Detecta colisões significativas
- **Velocidade Angular**: 300 °/s - Indica rotação rápida típica de queda
- **Tempo de Confirmação**: 5 amostras (50ms) - Evita falsos positivos

### 2.3 Sensor de Velocidade (Reed-Switch)

O Reed-Switch é um sensor magnético simples que detecta a passagem de um ímã próximo.

| Especificação | Valor | Unidade | Observações |
|---|---|---|---|
| **Tipo de Chave** | Magnética (Reed-Switch) | - | Sem partes móveis |
| **Tensão de Operação** | 3.3 a 5.0 | V | Compatível com GPIO do ESP32 |
| **Corrente Máxima** | 0.5 | A | Suficiente para GPIO |
| **Tempo de Resposta** | < 1 | ms | Muito rápido para detecção de rotação |
| **Vida Útil** | > 10 milhões | ciclos | Extremamente durável |
| **Temperatura Operacional** | -40 a +85 | °C | Ampla faixa de operação |
| **Imã Necessário** | Neodímio 5x5mm | - | Força magnética adequada |
| **Distância de Ativação** | 5 a 15 | mm | Depende da força do ímã |

**Aplicação no Projeto**: Dois Reed-Switches são utilizados - um no garfo (para medir velocidade) e outro no pedivela (para medir cadência).

#### 2.3.1 Cálculo de Velocidade

```cpp
// Cálculo de velocidade baseado em rotações de roda
class SpeedSensor {
private:
    float wheelCircumference;  // em metros
    volatile unsigned long pulseCount;
    unsigned long lastPulseTime;
    
public:
    float calculateSpeed() {
        // Número de pulsos = número de rotações da roda
        // Distância = pulsos × circunferência
        // Velocidade = distância / tempo
        
        unsigned long currentTime = millis();
        unsigned long timeDelta = currentTime - lastPulseTime;
        
        if (timeDelta > 0) {
            float distanceMeters = pulseCount * wheelCircumference;
            float timeSeconds = timeDelta / 1000.0;
            float speedMs = distanceMeters / timeSeconds;
            float speedKmh = speedMs * 3.6;  // Converter m/s para km/h
            
            return speedKmh;
        }
        return 0;
    }
};
```

**Calibração de Circunferência de Roda**:
- Pneu 700x23c: ~2.08 m
- Pneu 700x25c: ~2.10 m
- Pneu 700x28c: ~2.13 m
- Pneu 700x32c: ~2.17 m

### 2.4 Display OLED SSD1306

O display OLED monocromático oferece excelente visibilidade e baixo consumo de energia.

| Especificação | Valor | Unidade | Observações |
|---|---|---|---|
| **Tamanho da Tela** | 0.96 | polegadas | Diagonal |
| **Resolução** | 128 x 64 | pixels | Suficiente para dados numéricos |
| **Tipo de Display** | OLED | - | Emissão própria de luz |
| **Cor** | Monocromático (Branco) | - | Excelente contraste |
| **Tempo de Resposta** | < 1 | µs | Praticamente instantâneo |
| **Ângulo de Visão** | 160 | graus | Visível de múltiplos ângulos |
| **Consumo de Corrente** | 20 a 40 | mA | Depende do conteúdo exibido |
| **Protocolo** | I2C/SPI | - | Utilizamos I2C |
| **Endereço I2C** | 0x3C ou 0x3D | - | Configurável |
| **Tensão de Operação** | 3.3 a 5.0 | V | Compatível com ESP32 |

---

## 3. Limiares de Alarme e Detecção

### 3.1 Limiares de Inclinação (Gradient)

A inclinação da via é um fator crítico para a segurança e performance do ciclista. Diferentes limiares são utilizados para diferentes propósitos.

| Classificação | Gradiente | Descrição | Recomendação | Alarme |
|---|---|---|---|---|
| **Plano** | 0-2% | Via praticamente horizontal | Ideal para iniciantes | Nenhum |
| **Leve** | 2-4% | Pequena subida, confortável | Bom para treino | Nenhum |
| **Moderado** | 4-6% | Subida notável, requer esforço | Desafiador | Aviso (opcional) |
| **Íngreme** | 6-8% | Subida significativa | Muito desafiador | Aviso |
| **Muito Íngreme** | 8-12% | Subida severa | Extremamente desafiador | Alarme |
| **Extremo** | > 12% | Praticamente impossível para maioria | Risco de segurança | Alarme Crítico |

**Implementação em Código**:

```cpp
// Classificação de inclinação com alarmes
class GradientMonitor {
private:
    const float GRADIENT_MODERATE = 6.0;
    const float GRADIENT_STEEP = 8.0;
    const float GRADIENT_EXTREME = 12.0;
    
public:
    void checkGradientAlarm(float currentGradient) {
        if (currentGradient > GRADIENT_EXTREME) {
            // Alarme crítico
            buzzer.beep(5, 100);  // 5 bips rápidos
            leds.setColor(255, 0, 0);  // Vermelho
            display.showWarning("GRADIENTE EXTREMO!");
            
        } else if (currentGradient > GRADIENT_STEEP) {
            // Alarme de inclinação íngreme
            buzzer.beep(3, 150);  // 3 bips médios
            leds.setColor(255, 165, 0);  // Laranja
            display.showWarning("INCLINACAO ACENTUADA");
            
        } else if (currentGradient > GRADIENT_MODERATE) {
            // Aviso de inclinação moderada
            buzzer.beep(1, 200);  // 1 bip longo
            leds.setColor(255, 255, 0);  // Amarelo
        }
    }
};
```

### 3.2 Limiares de Aceleração e Detecção de Impacto

A detecção de impacto é crucial para alertar sobre possíveis quedas ou colisões.

| Tipo de Evento | Aceleração (g) | Giroscópio (°/s) | Duração | Ação |
|---|---|---|---|---|
| **Frenagem Normal** | 0.3-0.8 | < 50 | > 500ms | Nenhuma |
| **Frenagem Abrupta** | 0.8-1.5 | < 100 | 200-500ms | Nenhuma |
| **Impacto Leve** | 1.5-2.0 | 100-200 | < 200ms | Aviso |
| **Impacto Moderado** | 2.0-3.0 | 200-400 | < 200ms | Alarme |
| **Queda Detectada** | > 3.0 | > 400 | < 100ms | Alarme Crítico + SOS |

**Implementação de Detecção de Queda**:

```cpp
// Sistema de detecção de queda com confirmação
class CrashDetectionSystem {
private:
    const float IMPACT_THRESHOLD = 2.5;
    const float GYRO_THRESHOLD = 300.0;
    const int CONFIRMATION_SAMPLES = 3;
    int impactCounter = 0;
    
public:
    void processSensorData(float accelX, float accelY, float accelZ,
                          float gyroX, float gyroY, float gyroZ) {
        
        float accelMagnitude = sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ);
        float gyroMagnitude = sqrt(gyroX*gyroX + gyroY*gyroY + gyroZ*gyroZ);
        
        // Verificar critérios de impacto
        if (accelMagnitude > IMPACT_THRESHOLD && gyroMagnitude > GYRO_THRESHOLD) {
            impactCounter++;
            
            // Confirmar impacto após múltiplas amostras
            if (impactCounter >= CONFIRMATION_SAMPLES) {
                triggerCrashAlert();
                impactCounter = 0;
            }
        } else {
            impactCounter = 0;  // Reset se critérios não forem atendidos
        }
    }
    
    void triggerCrashAlert() {
        Serial.println("[CRASH] Queda detectada!");
        
        // Feedback imediato
        buzzer.beep(10, 50);  // Bips rápidos contínuos
        leds.setColor(255, 0, 0);  // Vermelho piscante
        display.showAlert("QUEDA DETECTADA!");
        
        // Registrar evento
        dataLogger.logCrashEvent(millis());
        
        // Aguardar confirmação do usuário
        // Se não confirmado em 30 segundos, enviar SOS (quando WiFi disponível)
    }
};
```

### 3.3 Limiares de Temperatura

A temperatura ambiente afeta o desempenho do ciclista e a segurança do equipamento.

| Faixa de Temperatura | Classificação | Recomendação | Alarme |
|---|---|---|---|
| < -10°C | Extremamente Frio | Risco de hipotermia | Aviso |
| -10 a 0°C | Muito Frio | Roupas adequadas necessárias | Aviso |
| 0 a 10°C | Frio | Roupas de inverno recomendadas | Nenhum |
| 10 a 20°C | Fresco | Roupas normais | Nenhum |
| 20 a 30°C | Confortável | Ideal para ciclismo | Nenhum |
| 30 a 40°C | Quente | Risco de desidratação | Aviso |
| > 40°C | Extremamente Quente | Risco de insolação | Alarme |

### 3.4 Limiares de Bateria

O gerenciamento de bateria é crítico para evitar perda de dados ou desligamento inesperado.

| Nível de Bateria | Tensão | Percentual | Ação |
|---|---|---|---|
| **Excelente** | 4.0-4.2V | 80-100% | Operação normal |
| **Bom** | 3.8-4.0V | 60-80% | Operação normal |
| **Adequado** | 3.5-3.8V | 30-60% | Operação normal com aviso |
| **Baixo** | 3.2-3.5V | 5-30% | Aviso frequente, reduzir funcionalidades |
| **Crítico** | 3.0-3.2V | 0-5% | Alarme contínuo, preparar desligamento |
| **Morto** | < 3.0V | 0% | Desligamento automático |

---

## 4. Análise Comparativa de Sensores Comerciais

### 4.1 Sensores Utilizados em Ciclocomputadores Premium

Os ciclocomputadores comerciais de ponta utilizam uma combinação sofisticada de sensores para fornecer dados altamente precisos.

| Sensor | Garmin Edge 850 | Hammerhead Karoo 3 | Wahoo ELEMNT BOLT V2 | Coros Dura Solar |
|---|---|---|---|---|
| **Receptor GNSS** | Multi-banda (L1/L5) | Multi-banda | Dual-band | Dual-band |
| **Acelerômetro** | 3-eixos | 3-eixos | 3-eixos | 3-eixos |
| **Giroscópio** | 3-eixos | 3-eixos | 3-eixos | 3-eixos |
| **Barômetro** | Sim (BMP280-like) | Sim | Sim | Sim |
| **Termômetro** | Sim | Sim | Sim | Sim |
| **Sensor de Luz** | Sim | Sim | Não | Sim (Solar) |
| **Bússola** | Sim | Sim | Não | Sim |
| **Sensor de Pressão Dinâmica** | Não | Não | Não | Não |

### 4.2 Protocolos de Comunicação com Sensores Externos

Os ciclocomputadores comerciais suportam múltiplos protocolos para conectar sensores periféricos.

| Protocolo | Frequência | Alcance | Consumo | Latência | Aplicação |
|---|---|---|---|---|---|
| **ANT+** | 2.4 GHz | ~10 metros | Muito baixo | < 200ms | Sensores de potência, cadência |
| **Bluetooth Low Energy** | 2.4 GHz | ~50 metros | Baixo | < 100ms | Sensores genéricos, smartphone |
| **WiFi (802.11b/g/n)** | 2.4/5 GHz | ~100 metros | Moderado | < 50ms | Sincronização de dados |
| **NFC** | 13.56 MHz | < 10cm | Muito baixo | Instantâneo | Autenticação, pagamento |

---

## 5. Implementação de Algoritmos Avançados

### 5.1 Fusão de Sensores (Sensor Fusion)

A combinação inteligente de dados de múltiplos sensores melhora significativamente a precisão das medições.

```cpp
// Algoritmo de fusão de sensores usando Filtro de Kalman simplificado
class SensorFusion {
private:
    float estimatedAltitude;
    float estimatedSpeed;
    float kalmanGain;
    
public:
    void updateAltitude(float barometerAltitude, float gpsAltitude) {
        // Pesar mais o barômetro (mais confiável para mudanças rápidas)
        // Usar GPS para correção de longo prazo
        
        kalmanGain = 0.7;  // 70% barômetro, 30% GPS
        
        estimatedAltitude = (kalmanGain * barometerAltitude) + 
                           ((1 - kalmanGain) * gpsAltitude);
    }
    
    void updateSpeed(float wheelSpeed, float gpsSpeed) {
        // Pesar mais a roda (mais responsiva)
        // Usar GPS para validação
        
        kalmanGain = 0.8;  // 80% roda, 20% GPS
        
        estimatedSpeed = (kalmanGain * wheelSpeed) + 
                        ((1 - kalmanGain) * gpsSpeed);
    }
};
```

### 5.2 Detecção de Anomalias

Identificar comportamentos anormais que podem indicar problemas.

```cpp
// Detector de anomalias em padrões de ciclismo
class AnomalyDetector {
public:
    bool detectAnomalousAcceleration(float currentAccel, float avgAccel) {
        // Se aceleração > 3 desvios padrão da média, é anomalia
        float deviation = abs(currentAccel - avgAccel);
        float threshold = 3.0 * calculateStdDev();
        
        return deviation > threshold;
    }
    
    bool detectStuckSensor(float sensorValue, int unchangedSamples) {
        // Se sensor não muda por 100 amostras, pode estar travado
        return unchangedSamples > 100;
    }
};
```

---

## 6. Validação e Testes

### 6.1 Protocolo de Testes de Bateria

```cpp
// Teste de autonomia de bateria
void testBatteryAutonomy() {
    Serial.println("=== TESTE DE AUTONOMIA DE BATERIA ===");
    
    unsigned long startTime = millis();
    float startVoltage = batteryMonitor.getBatteryVoltage();
    
    // Executar sistema em modo operacional completo
    while (batteryMonitor.getBatteryVoltage() > 3.0) {
        // Simular operação normal
        readAllSensors();
        updateDisplay();
        logData();
        delay(100);
    }
    
    unsigned long endTime = millis();
    float endVoltage = batteryMonitor.getBatteryVoltage();
    
    unsigned long totalMinutes = (endTime - startTime) / 60000;
    
    Serial.printf("Tempo de operação: %lu minutos\n", totalMinutes);
    Serial.printf("Tensão inicial: %.2f V\n", startVoltage);
    Serial.printf("Tensão final: %.2f V\n", endVoltage);
}
```

### 6.2 Validação de Sensores

```cpp
// Validação de precisão de sensores
void validateSensors() {
    Serial.println("=== VALIDAÇÃO DE SENSORES ===");
    
    // Teste BMP280
    float temp = bmp280.getTemperature();
    float alt = bmp280.getAltitude();
    Serial.printf("BMP280 - Temp: %.1f°C, Alt: %.0f m\n", temp, alt);
    
    // Teste MPU6050
    float accelX, accelY, accelZ;
    mpu6050.readAccel(accelX, accelY, accelZ);
    Serial.printf("MPU6050 - Accel: %.2f, %.2f, %.2f g\n", accelX, accelY, accelZ);
    
    // Teste Speed Sensor
    float speed = speedSensor.getSpeed();
    Serial.printf("Speed Sensor - Velocidade: %.1f km/h\n", speed);
}
```

---

## Referências

[1] Espressif Systems. "ESP32 Series Datasheet." Disponível em: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf

[2] Bosch Sensortec. "BMP280 Digital Pressure Sensor." Disponível em: https://www.bosch-sensortec.com/products/environmental-sensors/pressure-sensors/bmp280/

[3] InvenSense. "MPU-6050 Six-Axis (Gyro + Accelerometer) MEMS MotionTracking Device." Disponível em: https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000_DataSheet_en.pdf

[4] Solomon Systech. "SSD1306 Datasheet - Single-Chip CMOS OLED/PLED Driver with Controller." Disponível em: https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf

[5] Adafruit. "Lithium Ion Battery - 3.7V 2000mAh." Disponível em: https://www.adafruit.com/product/2011

[6] Chellal, A. A., Gonçalves, J., Lima, J., Pinto, V., & Megnafi, H. (2021). "Design of an embedded energy management system for Li–Po batteries based on a DCC-EKF approach for use in mobile robots." *Machines*, 9(12), 313.

[7] Hidayatullah, F. H., Abdurohman, M., et al. (2021). "Accident Detection System for Bicycle Athletes Using GPS/IMU Integration and Kalman Filtered AHRS Method." *IEEE Advancement in Data Science and Information Technology*, 9702085.

[8] Niittymäki, J. (2026). "Role of solar energy harvesting in hybrid energy solutions for wearable sports electronics." *Lappeenranta University of Technology*.

[9] Jannah, A. N., et al. (2025). "Cyclist safety in the digital age: A review of advanced warning technologies and systems." *Accident Analysis & Prevention*, 186, 107551.
