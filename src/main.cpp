#include <Arduino.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include "esp_heap_caps.h"

#include "bms_telemetry.h"
#include "ui_colors.h"
#include "ui_tabs.h"
#include "can_driver.h"
#include "simulator.h"

/* =========================================================================================
 * ESQUEMA DE PINOS:
 *  - Display SPI:  MOSI=23, MISO=19, SCK=18, CS=14, DC=2, RST=13
 *  - Touch SPI:    T_CS=27, T_CLK=18, T_DIN=23, T_DO=19
 *  - CAN (TWAI):   TX=GPIO 5, RX=GPIO 4 (MCP2542 Transceiver, 500 kbps)
 *  - I2C (Livre):  SDA=GPIO 21, SCL=GPIO 22 (MPU-6050)
 * ========================================================================================= */

#define MY_DISP_HOR_RES 320
#define MY_DISP_VER_RES 240

TFT_eSPI tft = TFT_eSPI();

/* Instância global da telemetria BMS */
BmsTelemetry bms;

/* =========================================================================================
 * DRIVERS LVGL (Display Flush & Touch Read)
 * ========================================================================================= */
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.pushImageDMA(area->x1, area->y1, w, h, (const uint16_t *)px_map);
    lv_display_flush_ready(disp);
}

void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
    tft.dmaWait();
    tft.endWrite();

    uint16_t touchX, touchY;
    bool touched = tft.getTouch(&touchX, &touchY);

    if (!touched) {
        data->state = LV_INDEV_STATE_RELEASED;
    } else {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = touchX;
        data->point.y = touchY;
    }
}

static uint32_t my_tick_get(void) {
    return millis();
}

/* =========================================================================================
 * SETUP & LOOP
 * ========================================================================================= */
void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n--- FSLART HANDKART BMS MONITOR UI ---");

    /* 1. Barramento CAN TWAI nativo */
    can_init();

    /* 2. Inicialização LVGL 9 */
    lv_init();
    lv_tick_set_cb(my_tick_get);

    /* 3. Display TFT e Touch XPT2046 */
    tft.init();
    tft.setRotation(1); /* 1 = Paisagem (320x240) */

    // Ativa o motor DMA do ESP32 para SPI assíncrono de alta velocidade
    tft.initDMA();

    // Calibração padrão de touch para ILI9341 em rotação 1
    uint16_t calData[5] = { 275, 3620, 264, 3532, 1 };
    tft.setTouch(calData);

    /* 4. Criar Display LVGL v9 com Double Buffering e DMA */
    lv_display_t *disp = lv_display_create(MY_DISP_HOR_RES, MY_DISP_VER_RES);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565_SWAPPED);

    const size_t buf_size = MY_DISP_HOR_RES * 40 * sizeof(uint16_t);
    uint16_t *buf1 = (uint16_t *)heap_caps_malloc(buf_size, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    uint16_t *buf2 = (uint16_t *)heap_caps_malloc(buf_size, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);

    if (buf1 && buf2) {
        lv_display_set_buffers(disp, buf1, buf2, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
        Serial.println("[DMA] 2 Buffers de 40 linhas ativos na SRAM interna!");
    } else {
        static uint16_t s_buf[MY_DISP_HOR_RES * 20];
        lv_display_set_buffers(disp, s_buf, NULL, sizeof(s_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
        Serial.println("[DMA] Fallback para buffer estático simples.");
    }
    lv_display_set_flush_cb(disp, my_disp_flush);

    /* 5. Registar Touchscreen Input Pointer */
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, my_touchpad_read);
    lv_indev_set_display(indev, disp);

    /* 6. Construir Interface com Abas Deslizantes */
    build_ui();
    update_ui(); // Primeira pintura imediata

    Serial.println("Display e UI inicializados. Pronto!");
}

void loop() {
    uint32_t time_till_next = lv_timer_handler();
    can_receive_task();

    static uint32_t last_sim_ms = 0;
    if (millis() - last_sim_ms >= 200) { // 5 Hz refresh
        last_sim_ms = millis();
        sim_tick();
        update_ui();
    }

    if (time_till_next > 10) time_till_next = 10;
    delay(time_till_next > 0 ? 1 : 0);
}
