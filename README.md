# Handkart Controller Display

Interface de monitorização do BMS do Handkart para ESP32, com display TFT SPI
ILI9341/XPT2046 e LVGL.

## Estado do projeto

A UI está funcional e inclui:

- Monitorização de tensão, corrente, potência, SOC e temperaturas.
- Estado do SDC, contactores, precharge e slaves.
- Informação da sessão de carregamento.
- Modo DEMO para testar a interface sem CAN ligado.
- Navegação entre quatro ecrãs através dos botões laterais.
- Receção de mensagens CAN através do controlador TWAI do ESP32 a 500 kbps.

O código já foi separado por blocos principais, mas ainda necessita de
organização e refatoração adicional. A UI, telemetria, CAN e simulador estão
em módulos distintos, embora ainda existam melhorias futuras de estrutura e
testes.

## Hardware

- ESP32 Dev Module.
- Display TFT 2.4" SPI, controlador ILI9341, resolução 320x240.
- Touch XPT2046.
- Transceiver CAN MCP2542.
- CAN TX: GPIO 5.
- CAN RX: GPIO 4.
- Display SPI: MOSI 23, MISO 19, SCK 18, CS 14, DC 2, RST 13.
- Touch CS: GPIO 27.

## Requisitos

- Visual Studio Code com PlatformIO.
- ESP32 Arduino framework.
- Cabo USB para compilar e fazer upload.

As dependências são instaladas automaticamente pelo PlatformIO através de
`platformio.ini`:

- `TFT_eSPI`
- `lvgl`

## Compilar e carregar

Na raiz do projeto:

```bash
pio run
pio run --target upload
pio device monitor -b 115200
```

Também é possível executar os comandos pela interface do PlatformIO no
Visual Studio Code.

## Branch de desenvolvimento

Esta versão está disponível no branch:

`feature/handkart-lvgl-ui`

## Estrutura principal

- `src/main.cpp`: inicialização do hardware, display LVGL e ciclo principal.
- `src/ui_tabs.cpp`: construção da interface e atualização dos ecrãs.
- `src/can_driver.cpp`: inicialização e descodificação do CAN/TWAI.
- `src/simulator.cpp`: dados simulados do modo DEMO.
- `include/`: interfaces dos módulos, modelo de telemetria, cores e callbacks.
- `platformio.ini`: placa, framework, dependências e configuração dos pinos.

## Notas

O modo DEMO inicia ativo e é desativado quando são recebidas mensagens CAN.
Antes de ligar o sistema real, confirmar os pinos, velocidade CAN e o formato
das mensagens utilizado pelo BMS.
