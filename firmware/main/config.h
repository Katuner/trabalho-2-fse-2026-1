#ifndef CONFIG_H
#define CONFIG_H

// ============ CONFIGURAÇÕES DE HARDWARE ============
#define PIN_I2C_SDA             21
#define PIN_I2C_SCL             22
#define PIN_SPEED_SENSOR        25
#define PIN_CADENCE_SENSOR      26
#define PIN_BUZZER              13
#define PIN_LED_R               14
#define PIN_LED_G               27
#define PIN_LED_B               12
#define PIN_BUTTON_START        32
#define PIN_BUTTON_LAP          33
#define PIN_BUTTON_MENU         35

// I2C Addresses
#define I2C_ADDR_BMP280         0x76
#define I2C_ADDR_MPU6050        0x68
#define I2C_ADDR_OLED           0x3C

// ============ CONFIGURAÇÕES DE WIFI E MQTT ============
#define WIFI_SSID               "BordoPC"
#define WIFI_PASS               "123456"
#define MQTT_BROKER_URL         "mqtt://tb.fse.lappis.rocks"
#define MQTT_ACCESS_TOKEN       "3N4Y4hb6uSpRTS6VP3Fg"

// ============ CONFIGURAÇÕES DE SENSORES ============
#define WHEEL_CIRCUMFERENCE     2.1f
#define DEBOUNCE_TIME_MS        50

#endif // CONFIG_H
