# Arquitetura FreeRTOS no ESP-IDF

## 1. Organização das Tarefas (Tasks)
Diferente da versão Arduino, a implementação em ESP-IDF utiliza as APIs nativas do FreeRTOS integradas ao framework.

| Task | Prioridade | Descrição |
|---|---|---|
| `MqttTask` | 5 | Gerencia a conexão e publicação de telemetria. |
| `SensorTask` | 4 | Leitura periódica de sensores I2C. |
| `SpeedTask` | 6 | Processamento de alta prioridade para pulsos de velocidade. |
| `DisplayTask` | 3 | Atualização da interface visual. |

## 2. Comunicação Inter-Tasks
- **Queues:** Utilizadas para passar structs de dados entre as tarefas de sensores e a tarefa de comunicação.
- **Semaphores (Mutex):** Garantem que apenas uma tarefa acesse o barramento I2C por vez.
- **ISRs:** As interrupções de GPIO notificam a `SpeedTask` usando `vTaskNotifyGiveFromISR`.

## 3. Benefícios
Esta estrutura garante que a telemetria via WiFi não bloqueie a leitura crítica de sensores ou o processamento de pulsos de velocidade, mantendo a precisão do computador de bordo.
