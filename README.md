# Sistema de Monitoramento com Processamento Dual-Core

## Objetivo
Este projeto implementa um sistema de monitoramento ambiental utilizando o microcontrolador Raspberry Pi Pico, aproveitando sua capacidade de processamento dual-core. O sistema realiza a leitura dos sensores BMP280 e BH1750 e apresenta as informações em um display OLED, com sistema de alertas visuais e sonoros.

## Ferramentas Utilizadas

### Hardware
- Raspberry Pi Pico (RP2040)
- Sensor BMP280 (temperatura e pressão atmosférica)
- Sensor BH1750 (luminosidade)
- Display OLED SSD1306
- LEDs (vermelho e azul)
- Buzzer
- Componentes de interface I2C

### Software e Desenvolvimento
- CMake (sistema de build)
- Pico SDK
- Linguagem C
- Protocolo I2C para comunicação com sensores e display
- Sistema de processamento multi-core

## Funcionalidades

### Core 0 (Aquisição de Dados)
- Leitura contínua dos sensores BMP280 e BH1750
- Processamento e conversão dos dados brutos
- Comunicação inter-core através de FIFO
- Amostragem a cada 1 segundo

### Core 1 (Interface do Usuário)
- Exibição dos dados no display OLED
- Sistema de alerta visual (LED vermelho/azul)
- Alerta sonoro através do buzzer
- Monitoramento de condições críticas:
  - Temperatura acima de 36°C
  - Iluminância abaixo de 10 lux

## Como Executar o Projeto

### Pré-requisitos
- Raspberry Pi Pico SDK instalado
- CMake
- Compilador ARM GCC

### Passos para Compilação

1. Clone o repositório:
```bash
git clone [URL_DO_REPOSITORIO]
cd dual-core
```

2. Crie e entre no diretório de build:
```bash
mkdir build
cd build
```

3. Configure o projeto com CMake:
```bash
cmake ..
```

4. Compile o projeto:
```bash
make
```

5. Após a compilação, você encontrará o arquivo `.uf2` no diretório `build`. Para programar o Raspberry Pi Pico:
   - Conecte o Pico ao computador mantendo o botão BOOTSEL pressionado
   - Copie o arquivo `.uf2` para o volume do raspberry pi
   - O Pico reiniciará automaticamente com o novo programa
