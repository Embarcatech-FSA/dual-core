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
