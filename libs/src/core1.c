#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "ssd1306.h"
#include "core1.h"
#include "config.h"
#include <stdio.h>
#include "hardware/pwm.h" 


/**
 * @brief Configura o pino do buzzer para operar com PWM.
 * 
 */
void setup_buzzer() {

    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 125.0); 
    pwm_config_set_wrap(&config, 1000); 
    pwm_init(slice_num, &config, true);
}

/**
 * @brief Liga ou desliga o buzzer.
 * 
 * @param on true para ligar, false para desligar.
 */
void set_buzzer_state(bool on) {
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    if (on) {
        pwm_set_gpio_level(BUZZER_PIN, 500); 
    } else {
        pwm_set_gpio_level(BUZZER_PIN, 0); 
    }
}



void core1_entry() {
    ssd1306_t ssd;

    // --- INICIALIZAÇÃO DO BUZZER ---
    setup_buzzer();
    set_buzzer_state(false);

    // Variáveis de estado para o alarme não-bloqueante
    bool in_alert_state = false;
    bool buzzer_is_on = false;
    uint64_t last_toggle_time = 0;
    uint32_t BEEP_INTERVAL_MS = 200;
    uint32_t atualizacoes_display = 0;

    gpio_init(LED_RED);
    gpio_init(LED_BLUE);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_set_dir(LED_BLUE, GPIO_OUT);


    // --- INICIALIZAÇÃO DO DISPLAY OLED ---
    i2c_init(I2C_PORT_SSD1306, 400 * 1000);
    gpio_set_function(I2C_SDA_SSD1306, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_SSD1306, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_SSD1306);
    gpio_pull_up(I2C_SCL_SSD1306);

    ssd1306_init(&ssd, DISPLAY_WIDTH, DISPLAY_HEIGHT, false, DISPLAY_ADDRESS, I2C_PORT_SSD1306);
    ssd1306_config(&ssd);
    
    // Mostra mensagem inicial e aguarda um pouco
    ssd1306_fill(&ssd, false);
    ssd1306_draw_string(&ssd, "Iniciando...", 18, 28);
    ssd1306_send_data(&ssd);
    sleep_ms(2000); // Aguarda 2 segundos para que a mensagem seja visível



    while (true) {
        // Verifica se há novos dados e atualiza o estado do alerta
        if (multicore_fifo_rvalid()) {
            uint32_t ptr = multicore_fifo_pop_blocking();
            sensor_data_t* dados = (sensor_data_t*)ptr;
            uint32_t tempo_processamento = to_ms_since_boot(get_absolute_time());
            uint32_t latencia = tempo_processamento - dados->timestamp_leitura;
            
            atualizacoes_display++;
            
            printf("Core 1 - Escrita %lu:\n", atualizacoes_display);
            printf("Tempo desde leitura: %lu ms\n", latencia);
            printf("Temperatura: %.2f C\n", dados->temperatura);
            printf("Iluminância: %.2f lux\n", dados->iluminancia);
            printf("Pressão: %.2f hPa\n", dados->pressao);

            // Atualiza a flag de estado de alerta
            if (dados->temperatura > 36.0f || dados->iluminancia < 10.0f) {
                if (!in_alert_state) {
                    printf("\nALERTA: Condição crítica detectada!\n");
                    in_alert_state = true;
                }
            } else {
                in_alert_state = false;
            }


            // Limpa o buffer do display antes de desenhar
            ssd1306_fill(&ssd, false);

            // Cria um buffer para formatar as strings
            char line_buffer[22];

            if (in_alert_state) {
                // TELA DE ALERTA
                ssd1306_draw_string(&ssd, "--- ALERTA ---", 8, 5); // Centraliza a mensagem de alerta

                // Verifica qual condição causou o alerta para exibi-la
                if (dados->temperatura > 35.0f) {
                    sprintf(line_buffer, "Temp Alta: %.1fC", dados->temperatura);
                    ssd1306_draw_string(&ssd, line_buffer, 0, 25);
                }
                if (dados->iluminancia < 100.0f) {
                    sprintf(line_buffer, "Luz Baixa: %.0f lx", dados->iluminancia);
                    ssd1306_draw_string(&ssd, line_buffer, 0, 40);
                }
                // Acende o LED vermelho e apaga o azul
                gpio_put(LED_RED, 1);
                gpio_put(LED_BLUE, 0);
            } else {
                // TELA DE DADOS NORMAIS
                // Formata e desenha a linha da Temperatura
                sprintf(line_buffer, "Temp: %.1f C", dados->temperatura);
                ssd1306_draw_string(&ssd, line_buffer, 0, 5);

                // Formata e desenha a linha da Iluminância
                sprintf(line_buffer, "Luz:  %.0f lux", dados->iluminancia);
                ssd1306_draw_string(&ssd, line_buffer, 0, 25);

                // Formata e desenha a linha da Pressão
                sprintf(line_buffer, "Pres: %.0f hPa", dados->pressao);
                ssd1306_draw_string(&ssd, line_buffer, 0, 45);

                // Acende o LED azul e apaga o vermelho
                gpio_put(LED_RED, 0);
                gpio_put(LED_BLUE, 1);
            }

            // Envia o buffer atualizado para o display
            ssd1306_send_data(&ssd);
        }

        multicore_fifo_clear_irq(); // Limpa a interrupção da FIFO

        // Gerencia o estado do buzzer de forma não-bloqueante
        if (in_alert_state) {
            uint64_t now = time_us_64();
            if ((now - last_toggle_time) / 1000 > BEEP_INTERVAL_MS) {
                buzzer_is_on = !buzzer_is_on;
                set_buzzer_state(buzzer_is_on);
                last_toggle_time = now;
            }
        } else {
            if (buzzer_is_on) {
                buzzer_is_on = false;
                set_buzzer_state(false);
            }
        }
    }
}