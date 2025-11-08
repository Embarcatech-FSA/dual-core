#ifndef CONFIG_H
#define CONFIG_H

// Estrutura de dados compartilhada entre os núcleos
typedef struct {
    float temperatura;
    float pressao;
    float iluminancia;
    uint32_t timestamp_leitura;    // Timestamp quando os dados foram lidos
} sensor_data_t;

/* VARIÁVEIS USADAS NO CORE 0 */
#define I2C_PORT_SENSORES i2c0
#define I2C_SDA_SENSORES 0
#define I2C_SCL_SENSORES 1
#define SEA_LEVEL_PRESSURE 101725.0f // Pressão ao nível do mar em Pa


/* VARIÁVEIS USADAS NO CORE 1 */
#define I2C_PORT_SSD1306 i2c1
#define I2C_SDA_SSD1306 14
#define I2C_SCL_SSD1306 15
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_ADDRESS 0x3C
#define LED_RED 13
#define LED_BLUE 12
#define BUZZER_PIN 10

#endif // CONFIG_H