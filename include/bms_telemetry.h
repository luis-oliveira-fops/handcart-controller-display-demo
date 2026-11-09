#ifndef BMS_TELEMETRY_H
#define BMS_TELEMETRY_H

#include <Arduino.h>
#include <stdint.h>

/* =========================================================================================
 * MODELO DE DADOS UNIVERSAL BMS (FSLART UniversalBmsState)
 * ========================================================================================= */
struct BmsTelemetry {
    // Pack Principal (Fonte: ISA IVT 0x521, 0x522, 0x526, 0x528)
    float pack_voltage = 538.4f;       // V
    float pack_current = 0.0f;         // A (Positivo = descarga, Negativo = carga)
    float pack_power = 0.0f;           // kW
    float energy_used_kwh = 4.12f;     // kWh
    float soc = 86.5f;                 // % (0..100)

    // Células e Temperatura (Fonte: Master MSC 3 0x704 / 1796)
    float cell_v_max = 4.172f;         // V
    float cell_v_min = 4.138f;         // V
    float cell_v_delta_mv = 34.0f;     // mV
    float temp_max = 34.8f;            // °C
    float temp_min = 23.4f;            // °C
    float temp_mcu = 38.5f;            // °C (Master MSC 1 0x701)
    uint8_t fan_pwm = 60;              // %

    // Segmentos (S1 a S6, ~90V cada em 144s)
    float seg_voltage[6] = { 89.8f, 89.7f, 89.9f, 89.6f, 89.7f, 89.7f };

    // Segurança & Contactores (0x702, 0x709)
    bool sdc_closed = true;            // Shutdown Circuit Loop
    bool ams_ok = true;                // AMS OK Signal
    bool air_pos = true;               // AIR+ Contactor
    bool air_neg = true;               // AIR- Contactor
    bool precharge_done = true;        // Precharge completado
    char master_state[16] = "HV_ON";   // Estado da FSM do Master
    uint8_t slaves_detected = 12;      // Total 12 slaves

    // Carregador / Handkart (0x18FF50E8 / Handkart session)
    bool charger_connected = true;
    float charger_v_req = 588.0f;      // V alvo
    float charger_i_req = 12.0f;       // A limite
    float session_kwh = 2.45f;         // kWh entregues nesta sessão
    uint32_t session_seconds = 1840;   // Duração da sessão

    // Diagnóstico
    bool demo_mode = true;             // Inicia em modo DEMO até receber tramas reais
    uint32_t can_rx_total = 0;
    uint32_t last_can_rx_ms = 0;
};

extern BmsTelemetry bms;

#endif // BMS_TELEMETRY_H
