## Implementação do Computador de Bordo com ESP-IDF e FreeRTOS

### 1. Transição de Paradigma: Do Arduino ao ESP-IDF

A implementação do computador de bordo para ciclistas passou por uma reestruturação fundamental, migrando do ambiente de desenvolvimento Arduino para o **ESP-IDF (Espressif IoT Development Framework)**. Esta transição foi motivada pela necessidade de maior controle sobre os recursos do microcontrolador ESP32, otimização de desempenho e aderência aos requisitos de um sistema embarcado profissional.

O ESP-IDF oferece acesso direto às APIs de baixo nível, permitindo uma gestão mais granular de hardware e software. Consequentemente, a utilização do **FreeRTOS**, que é nativamente integrado ao ESP-IDF, tornou-se central para a arquitetura de software, possibilitando a criação de um sistema multitarefa robusto e responsivo.

### 2. Arquitetura de Software Baseada em FreeRTOS

A arquitetura foi concebida em torno do FreeRTOS, dividindo as funcionalidades do sistema em tarefas (Tasks) concorrentes. Esta abordagem garante que operações críticas, como a leitura de sensores, não sejam comprometidas por tarefas de menor prioridade, como a comunicação de rede.

#### 2.1. Visão Geral das Tarefas

| Tarefa (`Task`) | Prioridade | Descrição Detalhada |
| --- | --- | --- |
| **SpeedTask** | 6 | Processa pulsos do sensor de efeito Hall via interrupção (ISR) para calcular velocidade e distância. |
| **AlarmTask** | 6 | Monitora em tempo real se a velocidade ou temperatura excedem os limites configurados via Thingsboard. |
| **SensorTask** | 4 | Realiza a leitura periódica dos sensores I2C (BMP280 para temperatura/altitude e MPU6050 para inclinação). |
| **MqttTask** | 5 | Gerencia a publicação de telemetria e a recepção de comandos RPC e Atributos do Thingsboard. |
| **DisplayTask** | 3 | Atualiza a interface visual no display OLED com os dados mais recentes das filas. |

### 3. Comunicação Wireless e Controle Bidirecional

Um dos pilares deste projeto é a integração com a plataforma **Thingsboard** utilizando o protocolo **MQTT**. A solução implementada não é apenas um monitor de dados, mas um sistema de controle bidirecional completo.

#### 3.1. Telemetria (ESP32 -> Nuvem)

A ESP32 publica um payload JSON contendo:

- `velocidade`: Velocidade instantânea (km/h).

- `distancia`: Distância total percorrida (km).

- `temperatura`: Temperatura ambiente (°C).

- `alarme`: Estado do sistema de alerta (booleano).

#### 3.2. Controle e Configuração (Nuvem -> ESP32)

- **RPC (Remote Procedure Call)**: Através de um widget de switch no celular, o usuário pode enviar o comando `set_display` para ligar ou desligar a interface da placa remotamente.

- **Atributos Compartilhados**: O usuário define limites de segurança (`limiteVelocidade` e `limiteTemperatura`) no dashboard. A ESP32 recebe essas atualizações e ajusta o comportamento do alarme (Buzzer) em tempo real, sem necessidade de reinicialização.

### 4. Conclusão

A migração para o ESP-IDF e a implementação de uma arquitetura multitarefa com FreeRTOS elevaram a maturidade técnica do projeto. A integração com o Thingsboard via MQTT cumpre integralmente os requisitos de comunicação wireless e controle remoto, resultando em um produto de sistemas embarcados robusto e alinhado com as práticas da indústria.