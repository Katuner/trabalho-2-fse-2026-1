import paho.mqtt.client as mqtt
import time
import random

# CONFIGURAÇÃO
THINGSBOARD_HOST = 'tb.fse.lappis.rocks'
ACCESS_TOKEN = 'sINHm4Z1o6MvERK18xZG'

client = mqtt.Client()
client.username_pw_set(ACCESS_TOKEN)
client.connect(THINGSBOARD_HOST, 1883, 60)
client.loop_start()

print("Simulando ESP32... Olhe para o seu Dashboard!")

try:
    distancia = 0
    while True:
        velocidade = random.uniform(15, 30)
        distancia += velocidade / 3600
        payload = {
            "velocidade": round(velocidade, 2),
            "temperatura": round(random.uniform(22, 28), 2),
            "distancia": round(distancia, 2),
            "altitude": round(random.uniform(100, 110), 2),
            "bateria": 85
        }
        client.publish('v1/devices/me/telemetry', str(payload).replace("'", '"'))
        print(f"Enviado: {payload}")
        time.sleep(2)
except KeyboardInterrupt:
    client.disconnect()
