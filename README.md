# Trabalho Final - Fundamentos de Sistemas Embarcados (2026/1)

Este repositório contém a evolução do Trabalho 2 para o Trabalho Final da disciplina de Fundamentos de Sistemas Embarcados (FSE) — UnB/FGA, semestre 2026/1.

## Objetivo
O projeto original foi migrado de Arduino para o framework **ESP-IDF** nativo, utilizando **FreeRTOS** para gerenciamento de tarefas e comunicação **Wireless via MQTT** com a plataforma **Thingsboard**.

## Principais Mudanças
1. **Framework:** Migração completa de Arduino C++ para ESP-IDF C.
2. **Concorrência:** Uso de Tasks do FreeRTOS, Filas (Queues) e Semáforos Mutex para gerenciar sensores e telemetria.
3. **Comunicação:** Implementação de telemetria e controle bidirecional (RPC) via MQTT com o Thingsboard.
4. **Hardware:** Suporte a sensores BMP280, MPU6050, Display OLED SSD1306 e Reed-Switches.

## Estrutura do Projeto
- `firmware/`: Código fonte em ESP-IDF.
- `ComputadorBordoCiclista/`: Documentação atualizada e especificações técnicas.

## Alunos
| Nome completo | Matrícula |
|---|---|
| Lucas Oliveira Meireles | 190016647 |
| Pedro Ramos Sousa Reis | 222031680 |

---
*Este trabalho foi corrigido para atender aos requisitos de uso de ESP-IDF e comunicação Wireless.*
