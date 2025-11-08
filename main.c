/**
 * Sistema de monitoramento e interface com dois núcleos de processamento.
 * Core 0 é responsável pela aquisição contínua e processamento de dados de dois sensores: BMP280 e BH1750.
 * Core 1 é responsável pela Interface de Usuário.
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "bmp280.h"
#include "bh1750.h"
#include <math.h>
#include "pico/multicore.h"
#include "core1.h"
#include "config.h"



int32_t raw_temp_bmp;
int32_t raw_pressure;
struct bmp280_calib_param params;

int main()
{
    stdio_init_all();
    multicore_launch_core1(core1_entry);
    
    i2c_init(I2C_PORT_SENSORES, 400 * 1000);
    gpio_set_function(I2C_SDA_SENSORES, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_SENSORES, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_SENSORES);
    gpio_pull_up(I2C_SCL_SENSORES);

    bmp280_init(I2C_PORT_SENSORES);
    bmp280_get_calib_params(I2C_PORT_SENSORES, &params);

    bh1750_power_on(I2C_PORT_SENSORES);
    static sensor_data_t dados;
    uint32_t leituras_realizadas = 0;

    while (true) {
        // Registra timestamp antes da leitura
        uint32_t tempo_inicial = to_ms_since_boot(get_absolute_time());
        
        // Core 0 coleta os dados dos sensores
        bmp280_read_raw(I2C_PORT_SENSORES, &raw_temp_bmp, &raw_pressure);
        int32_t temperature = bmp280_convert_temp(raw_temp_bmp, &params);
        int32_t pressure = bmp280_convert_pressure(raw_pressure, raw_temp_bmp, &params);

        uint16_t lux = bh1750_read_measurement(I2C_PORT_SENSORES);

        dados.temperatura = temperature / 100.0f; 
        dados.pressao = pressure / 100.0f;
        dados.iluminancia = lux;
        dados.timestamp_leitura = tempo_inicial;
        
        printf("\nCore 0 - Leitura %lu realizada em %lu ms\n", 
               ++leituras_realizadas, 
               to_ms_since_boot(get_absolute_time()) - tempo_inicial);
               
        multicore_fifo_push_blocking((uint32_t)&dados);  
        sleep_ms(1000);
    }
}
