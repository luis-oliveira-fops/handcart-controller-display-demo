#include <Arduino.h>
#include <string.h>

#include "bms_telemetry.h"
#include "can_driver.h"

void can_init() {
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK && twai_start() == ESP_OK) {
        Serial.println("[CAN] Barramento TWAI iniciado com sucesso a 500 kbps (TX=5, RX=4)");
    } else {
        Serial.println("[CAN] AVISO: Erro ao iniciar driver TWAI.");
    }
}

void can_receive_task() {
    twai_message_t rx_msg;
    while (twai_receive(&rx_msg, 0) == ESP_OK) {
        bms.can_rx_total++;
        bms.last_can_rx_ms = millis();
        bms.demo_mode = false;

        const uint32_t id = rx_msg.identifier;

        if (id == 0x521 && rx_msg.data_length_code >= 6) {
            int32_t raw_i = (int32_t)((rx_msg.data[2] << 24) | (rx_msg.data[3] << 16) |
                                      (rx_msg.data[4] << 8) | rx_msg.data[5]);
            bms.pack_current = raw_i / 1000.0f;
        } else if (id == 0x522 && rx_msg.data_length_code >= 6) {
            int32_t raw_u = (int32_t)((rx_msg.data[2] << 24) | (rx_msg.data[3] << 16) |
                                      (rx_msg.data[4] << 8) | rx_msg.data[5]);
            bms.pack_voltage = raw_u / 1000.0f;
            bms.pack_power = (bms.pack_voltage * bms.pack_current) / 1000.0f;
        } else if (id == 0x526 && rx_msg.data_length_code >= 6) {
            int32_t raw_w = (int32_t)((rx_msg.data[2] << 24) | (rx_msg.data[3] << 16) |
                                      (rx_msg.data[4] << 8) | rx_msg.data[5]);
            bms.pack_power = raw_w / 1000.0f;
        } else if (id == 0x704 && rx_msg.data_length_code >= 8) {
            uint16_t v_max = rx_msg.data[0] | (rx_msg.data[1] << 8);
            uint16_t t_max = rx_msg.data[2] | (rx_msg.data[3] << 8);
            uint16_t v_min = rx_msg.data[4] | (rx_msg.data[5] << 8);
            uint16_t t_min = rx_msg.data[6] | (rx_msg.data[7] << 8);

            bms.cell_v_max = v_max * 0.001f;
            bms.cell_v_min = v_min * 0.001f;
            bms.cell_v_delta_mv = (bms.cell_v_max - bms.cell_v_min) * 1000.0f;
            bms.temp_max = t_max * 0.01f;
            bms.temp_min = t_min * 0.01f;
        } else if (id == 0x705 && rx_msg.data_length_code >= 3) {
            uint8_t soc_int = rx_msg.data[0];
            uint16_t soc_dec = rx_msg.data[1] | (rx_msg.data[2] << 8);
            bms.soc = soc_int + (soc_dec * 0.01f);
        } else if (id == 0x701 && rx_msg.data_length_code >= 6) {
            bms.temp_mcu = (float)rx_msg.data[1];
            bms.fan_pwm = (uint8_t)(rx_msg.data[5] * 0.392f);
        } else if (id == 0x702 && rx_msg.data_length_code >= 2) {
            bms.air_pos = (rx_msg.data[0] & 0x01) != 0;
            bms.air_neg = (rx_msg.data[0] & 0x02) != 0;

            const uint8_t state = rx_msg.data[1];
            if (state == 4) {
                strcpy(bms.master_state, "HV_ON");
                bms.precharge_done = true;
            } else if (state == 1) {
                strcpy(bms.master_state, "PRECHARGE");
                bms.precharge_done = false;
            } else {
                strcpy(bms.master_state, "STANDBY");
                bms.precharge_done = false;
            }
        } else if (id == 0x709 && rx_msg.data_length_code >= 1) {
            bms.sdc_closed = rx_msg.data[0] != 0;
        }
    }
}
