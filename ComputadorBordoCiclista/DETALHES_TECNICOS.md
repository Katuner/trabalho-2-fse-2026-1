# Detalhes Técnicos: Arquitetura ESP-IDF e MQTT

## 1. Visão Geral do Sistema
O sistema agora opera sob o framework oficial da Espressif, o **ESP-IDF**, garantindo maior controle sobre os recursos do hardware e conformidade com os requisitos da disciplina.

## 2. Comunicação Wireless (MQTT)
A ESP32 conecta-se ao broker MQTT do Thingsboard (`tb.fse.lappis.rocks`) para:
- **Publicação de Telemetria:** Envio de velocidade, altitude, temperatura e dados inerciais em formato JSON.
- **Controle via RPC:** Recebimento de comandos remotos (Remote Procedure Call) para ações como resetar a viagem ou alternar telas do display.

### Tópicos MQTT
- **Telemetria:** `v1/devices/me/telemetry`
- **RPC Request:** `v1/devices/me/rpc/request/+`

## 3. Arquitetura FreeRTOS
As tarefas são divididas para garantir tempo real e eficiência:
- `MqttTask`: Gerencia a conexão WiFi e publica dados das filas.
- `SensorTask`: Realiza a leitura periódica dos sensores I2C.
- `SpeedTask`: Processa interrupções do sensor de velocidade.
- `DisplayTask`: Renderiza a interface no OLED.

## 4. Pinagem (GPIO)
Mantida a pinagem original para compatibilidade com o hardware montado:
- I2C (SDA: 21, SCL: 22)
- Velocidade: GPIO 25
- Cadência: GPIO 26
- Buzzer: GPIO 13
- Botões: 32, 33, 35
