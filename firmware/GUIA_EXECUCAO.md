# Guia de Execução e Dependências

Este guia detalha os passos necessários para configurar o ambiente e executar tanto o firmware na ESP32 quanto o script de simulação no computador.

## 1. Dependências do Firmware (ESP32)

Para compilar e carregar o código na placa, é necessário o framework oficial da Espressif.

- **ESP-IDF v5.x**: O projeto foi desenvolvido utilizando o **Espressif IoT Development Framework**.
  - [Guia de Instalação Oficial](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html)

- **Extensão VS Code (Opcional)**: Recomendamos a extensão "Espressif ESP-IDF" para facilitar o processo de Build, Flash e Monitor.

- **Driver USB-UART**: Certifique-se de ter os drivers CP210x ou CH340 instalados para que o computador reconheça a ESP32 via USB.

### Comandos de Compilação:

No diretório `firmware/`:

```bash
# Configurar o alvo para ESP32
idf.py set-target esp32

# Compilar, carregar e abrir o monitor serial
idf.py build flash monitor
```

---

## 2. Dependências da Simulação (Python)

Caso queira testar o Dashboard do Thingsboard sem o hardware físico, utilize o script de simulação.

- **Python 3.9 ou superior**: Certifique-se de que o Python está instalado e adicionado ao **PATH** do sistema.

- **Biblioteca Paho-MQTT**: Necessária para a comunicação via protocolo MQTT.

### Instalação da Biblioteca:

Abra o terminal e execute:

```
# No Windows
python -m pip install paho-mqtt

# No Linux/Mac
pip3 install paho-mqtt
```

---

## 3. Configurações Prévias

Antes de executar, você **DEVE** atualizar as seguintes informações no arquivo `firmware/main/config.h` (para a placa) ou no script Python (para a simulação):

1. **WIFI_SSID / WIFI_PASS**: Suas credenciais de internet.

1. **MQTT_ACCESS_TOKEN**: O Token do dispositivo gerado no seu Thingsboard.

1. **MQTT_BROKER_URL**: `mqtt://tb.fse.lappis.rocks`

---

## 4. Troubleshooting (Solução de Problemas)

- **Erro de 'pip' não reconhecido**: Utilize `python -m pip install` em vez de apenas `pip`.

- **ESP32 não conecta ao Wi-Fi**: Verifique se a rede é 2.4GHz (a ESP32 não suporta 5GHz).

- **Dados não aparecem no Dashboard**: Verifique se o Token no código é exatamente o mesmo que está no Thingsboard.