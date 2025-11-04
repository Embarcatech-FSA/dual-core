#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "ssd1306.h"
#include "core1.h"
#include "config.h"
#include <stdio.h>



ssd1306_t ssd;

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