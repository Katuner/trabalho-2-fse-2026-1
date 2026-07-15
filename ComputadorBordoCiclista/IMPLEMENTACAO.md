# Implementação: Computador de Bordo ESP-IDF

## 1. Mudança de Paradigma
A implementação foi migrada do ecossistema Arduino para o **ESP-IDF (Espressif IoT Development Framework)**. Isso permite:
- Uso nativo do **FreeRTOS** com maior controle de prioridades.
- Acesso direto às APIs de driver da Espressif (`driver/gpio.h`, `driver/i2c.h`).
- Implementação eficiente do cliente MQTT (`esp_mqtt`).

## 2. Componentes de Software
- **MQTT Client:** Utiliza o componente `esp_mqtt` para comunicação com o Thingsboard.
- **I2C Driver:** Gerenciamento do barramento I2C para sensores BMP280, MPU6050 e Display OLED.
- **GPIO & ISR:** Uso de interrupções de hardware para sensores de pulso (velocidade e cadência).

## 3. Fluxo de Dados
1. Os sensores são lidos por Tasks específicas.
2. Os dados são enviados via **Queues** para a `MqttTask`.
3. A `MqttTask` formata os dados em JSON e publica no tópico de telemetria.
4. Comandos RPC recebidos do Thingsboard são processados para ações de controle.
