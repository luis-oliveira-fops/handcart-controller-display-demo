#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#include <driver/twai.h>

/* Pinos CAN (TWAI ESP32 + MCP2542) */
#define CAN_TX_PIN GPIO_NUM_5
#define CAN_RX_PIN GPIO_NUM_4

/* Inicializa o barramento TWAI a 500 kbps */
void can_init(void);

/* Descodifica mensagens CAN conforme o descodificador T26 do repositório BMS_UI */
void can_receive_task(void);

#endif // CAN_DRIVER_H
