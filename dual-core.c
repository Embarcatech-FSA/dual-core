#include <stdio.h>
#include "pico/stdlib.h"
#include "bmp280.h"
#include "bh1750.h"
#include <math.h>
#include "pico/multicore.h"

#define I2C_PORT_SENSORES i2c0
#define I2C_SDA_SENSORES 0
#define I2C_SCL_SENSORES 1
#define SEA_LEVEL_PRESSURE 101725.0f // Pressão ao nível do mar em Pa

int32_t raw_temp_bmp;
int32_t raw_pressure;
struct bmp280_calib_param params;

// Estrutura de dados compartilhada entre os núcleos
typedef struct {
    float temperatura;
    float pressao;
    float iluminancia;
} sensor_data_t;


void core1_entry() {
    while (true) {
        if(multicore_fifo_rvalid()) {
            uint32_t ptr = multicore_fifo_pop_blocking(); // Recebe o ponteiro para os dados
            sensor_data_t* dados = (sensor_data_t*)ptr; // Converte o ponteiro de volta para a estrutura

            // Processa os dados recebidos do Core 0
            printf("Core 1 - Processando dados:\n");
            printf("Temperatura: %.2f C\n", dados->temperatura);
            printf("Pressão: %.2f hPa\n", dados->pressao);
            printf("Iluminância: %.2f lux\n", dados->iluminancia);

        }
        multicore_fifo_clear_irq(); // Limpa a interrupção da FIFO
    }
}

int main()
{
    stdio_init_all();
    
    i2c_init(I2C_PORT_SENSORES, 400 * 1000);
    gpio_set_function(I2C_SDA_SENSORES, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_SENSORES, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_SENSORES);
    gpio_pull_up(I2C_SCL_SENSORES);

    bmp280_init(I2C_PORT_SENSORES);
    bmp280_get_calib_params(I2C_PORT_SENSORES, &params);

    bh1750_power_on(I2C_PORT_SENSORES);

    multicore_launch_core1(core1_entry);
    static sensor_data_t dados;

    while (true) {
        // Core 0 coleta os dados dos sensores
        bmp280_read_raw(I2C_PORT_SENSORES, &raw_temp_bmp, &raw_pressure);
        int32_t temperature = bmp280_convert_temp(raw_temp_bmp, &params);
        int32_t pressure = bmp280_convert_pressure(raw_pressure, raw_temp_bmp, &params);

        uint16_t lux = bh1750_read_measurement(I2C_PORT_SENSORES);

        dados.temperatura = temperature / 100.0f;
        dados.pressao = pressure / 100.0f;
        dados.iluminancia = lux;
        multicore_fifo_push_blocking((uint32_t)&dados);  
        sleep_ms(1000);
    }
}
