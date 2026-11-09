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

O código ainda necessita de organização e refatoração. Neste momento a UI, a
telemetria e os drivers estão concentrados principalmente em
`src/main.cpp`, pelo que a separação por módulos deve ser feita numa fase
seguinte.

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

- `src/main.cpp`: aplicação, UI LVGL, modo DEMO e receção CAN.
- `platformio.ini`: placa, framework, dependências e configuração dos pinos.

## Notas

O modo DEMO inicia ativo e é desativado quando são recebidas mensagens CAN.
Antes de ligar o sistema real, confirmar os pinos, velocidade CAN e o formato
das mensagens utilizado pelo BMS.
