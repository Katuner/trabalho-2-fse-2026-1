# Guia de Execução: ESP-IDF

## 1. Pré-requisitos
- **ESP-IDF v5.x** instalado e configurado.
- Extensão do ESP-IDF para VS Code ou terminal com `export.sh`.

## 2. Configuração do Projeto
1. Navegue até a pasta `firmware/`.
2. Abra o arquivo `main/config.h` e insira suas credenciais de WiFi e o Access Token do seu dispositivo no Thingsboard.

## 3. Compilação e Flash
No terminal do ESP-IDF, execute:
```bash
idf.py build
idf.py -p <PORTA_SERIAL> flash monitor
```

## 4. Integração com Thingsboard
1. Acesse [https://tb.fse.lappis.rocks/](https://tb.fse.lappis.rocks/).
2. Crie um dispositivo do tipo "ESP32".
3. Copie o "Access Token" para o `config.h`.
4. Crie um Dashboard e adicione widgets de telemetria (Charts) e botões de RPC para controle.
