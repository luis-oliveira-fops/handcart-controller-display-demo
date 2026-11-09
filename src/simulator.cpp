#include <Arduino.h>
#include <math.h>

#include "bms_telemetry.h"
#include "simulator.h"

void sim_tick() {
    if (!bms.demo_mode) {
        return;
    }

    static float angle = 0.0f;
    angle += 0.08f;
    if (angle > 6.283f) {
        angle = 0.0f;
    }

    bms.pack_current = 15.0f + 25.0f * sinf(angle);
    bms.pack_voltage = 538.0f - (bms.pack_current * 0.08f);
    bms.pack_power = (bms.pack_voltage * bms.pack_current) / 1000.0f;

    bms.temp_max = 34.5f + 2.5f * sinf(angle * 0.5f);
    bms.temp_min = 23.2f + 1.2f * cosf(angle * 0.3f);
    bms.temp_mcu = 39.0f + 3.0f * sinf(angle * 0.4f);

    bms.cell_v_max = 4.165f + 0.015f * sinf(angle);
    bms.cell_v_min = 4.135f + 0.010f * cosf(angle);
    bms.cell_v_delta_mv = (bms.cell_v_max - bms.cell_v_min) * 1000.0f;

    bms.soc = 85.0f - (millis() / 60000.0f);
    if (bms.soc < 5.0f) {
        bms.soc = 98.0f;
    }

    for (int i = 0; i < 6; i++) {
        bms.seg_voltage[i] = (bms.pack_voltage / 6.0f) +
                             ((i % 2 == 0 ? 0.1f : -0.1f) * sinf(angle + i));
    }

    bms.session_seconds++;
    bms.session_kwh += 0.0001f;
}
