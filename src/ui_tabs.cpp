#include <Arduino.h>
#include <lvgl.h>
#include "bms_telemetry.h"
#include "ui_colors.h"
#include "ui_tabs.h"

/* =========================================================================================
 * ELEMENTOS GRÁFICOS LVGL (definições)
 * ========================================================================================= */
lv_obj_t *tv_main = NULL;
lv_obj_t *header_state_label;
lv_obj_t *header_can_label;
lv_obj_t *header_dot;

// Tab 1: Geral
lv_obj_t *lbl_pack_v;
lv_obj_t *lbl_pack_p;
lv_obj_t *lbl_pack_i;
lv_obj_t *lbl_temp_range;
lv_obj_t *lbl_soc_text;
lv_obj_t *bar_soc;

// Tab 2: Células
lv_obj_t *lbl_cell_max;
lv_obj_t *lbl_cell_min;
lv_obj_t *lbl_cell_delta;
lv_obj_t *lbl_seg_v[6];

// Tab 3: Sistema
lv_obj_t *lbl_sdc_status;
lv_obj_t *lbl_airs_status;
lv_obj_t *lbl_precharge_status;
lv_obj_t *lbl_mcu_info;
lv_obj_t *lbl_slaves_info;

// Tab 4: Handkart / Carregador
lv_obj_t *lbl_chg_status;
lv_obj_t *lbl_chg_target;
lv_obj_t *lbl_chg_session;
lv_obj_t *btn_demo_toggle;
lv_obj_t *lbl_demo_btn;

/* =========================================================================================
 * CALLBACKS DE NAVEGAÇÃO
 * ========================================================================================= */
static void tab_prev_cb(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED && tv_main != NULL) {
        uint32_t cur = lv_tabview_get_tab_active(tv_main);
        uint32_t target = (cur > 0) ? (cur - 1) : 3;
        lv_tabview_set_active(tv_main, target, LV_ANIM_OFF);
        update_ui();
    }
}

static void tab_next_cb(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED && tv_main != NULL) {
        uint32_t cur = lv_tabview_get_tab_active(tv_main);
        uint32_t target = (cur < 3) ? (cur + 1) : 0;
        lv_tabview_set_active(tv_main, target, LV_ANIM_OFF);
        update_ui();
    }
}

static void tabview_change_cb(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        update_ui();
    }
}

static void demo_btn_event_cb(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        bms.demo_mode = !bms.demo_mode;
        if (bms.demo_mode) {
            lv_label_set_text(lbl_demo_btn, "MODO DEMO: ATIVO");
            lv_obj_set_style_bg_color(btn_demo_toggle, COLOR_ACCENT, 0);
            lv_obj_set_style_text_color(lbl_demo_btn, lv_color_black(), 0);
        } else {
            lv_label_set_text(lbl_demo_btn, "MODO CAN REAL (AGUARDANDO)");
            lv_obj_set_style_bg_color(btn_demo_toggle, COLOR_SURFACE_2, 0);
            lv_obj_set_style_text_color(lbl_demo_btn, COLOR_TEXT_MAIN, 0);
        }
    }
}

/* Helper para criar cartões com estilo do BMS_UI */
static lv_obj_t* create_card(lv_obj_t *parent, int32_t w, int32_t h) {
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, w, h);
    lv_obj_set_style_bg_color(card, COLOR_SURFACE, 0);
    lv_obj_set_style_border_color(card, COLOR_BORDER, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_radius(card, 6, 0);
    lv_obj_set_style_pad_all(card, 6, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    return card;
}

/* =========================================================================================
 * CONSTRUÇÃO DA INTERFACE GRÁFICA (LVGL 9 TABVIEW COM SLIDE HORIZONTAL)
 * ========================================================================================= */
void build_ui() {
    lv_obj_t *screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, COLOR_BG, 0);

    /* 1. Barra de Cabeçalho Superior Fixa (Height 26px) */
    lv_obj_t *header = lv_obj_create(screen);
    lv_obj_set_size(header, 320, 26);
    lv_obj_set_style_bg_color(header, COLOR_SURFACE, 0);
    lv_obj_set_style_border_side(header, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_color(header, COLOR_BORDER, 0);
    lv_obj_set_style_border_width(header, 1, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_pad_all(header, 2, 0);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    // Ponto de Status Led
    header_dot = lv_obj_create(header);
    lv_obj_set_size(header_dot, 8, 8);
    lv_obj_set_style_radius(header_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(header_dot, COLOR_ACCENT, 0);
    lv_obj_set_style_border_width(header_dot, 0, 0);
    lv_obj_align(header_dot, LV_ALIGN_LEFT_MID, 6, 0);

    // Título Equipa
    lv_obj_t *logo_title = lv_label_create(header);
    lv_label_set_text(logo_title, "FSLART BMS");
    lv_obj_set_style_text_color(logo_title, COLOR_ACCENT, 0);
    lv_obj_set_style_text_font(logo_title, &lv_font_montserrat_12, 0);
    lv_obj_align(logo_title, LV_ALIGN_LEFT_MID, 20, 0);

    // Estado Central (Pill)
    header_state_label = lv_label_create(header);
    lv_label_set_text(header_state_label, "HV ON");
    lv_obj_set_style_text_color(header_state_label, COLOR_OK, 0);
    lv_obj_set_style_text_font(header_state_label, &lv_font_montserrat_12, 0);
    lv_obj_align(header_state_label, LV_ALIGN_CENTER, 0, 0);

    // Indicador CAN / DEMO à direita
    header_can_label = lv_label_create(header);
    lv_label_set_text(header_can_label, "DEMO");
    lv_obj_set_style_text_color(header_can_label, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(header_can_label, &lv_font_montserrat_12, 0);
    lv_obj_align(header_can_label, LV_ALIGN_RIGHT_MID, -6, 0);

    /* 2. Contentor Tabview com Gestos de Deslizar Horizontalmente */
    tv_main = lv_tabview_create(screen);
    lv_obj_add_event_cb(tv_main, tabview_change_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_tabview_set_tab_bar_position(tv_main, LV_DIR_BOTTOM);
    lv_tabview_set_tab_bar_size(tv_main, 36);
    lv_obj_set_size(tv_main, 320, 214);
    lv_obj_align(tv_main, LV_ALIGN_TOP_MID, 0, 26);
    lv_obj_set_style_bg_color(tv_main, COLOR_BG, 0);

    // Estilo da barra de abas
    lv_obj_t *tab_bar = lv_tabview_get_tab_bar(tv_main);
    lv_obj_set_style_bg_color(tab_bar, COLOR_SURFACE, 0);
    lv_obj_set_style_border_side(tab_bar, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_border_color(tab_bar, COLOR_BORDER, 0);
    lv_obj_set_style_border_width(tab_bar, 1, 0);
    lv_obj_set_style_pad_left(tab_bar, 54, 0);
    lv_obj_set_style_pad_right(tab_bar, 54, 0);
    lv_obj_set_style_pad_top(tab_bar, 0, 0);
    lv_obj_set_style_pad_bottom(tab_bar, 0, 0);

    // 4 Abas principais
    lv_obj_t *t1 = lv_tabview_add_tab(tv_main, "GERAL");
    lv_obj_t *t2 = lv_tabview_add_tab(tv_main, "CELULAS");
    lv_obj_t *t3 = lv_tabview_add_tab(tv_main, "SISTEMA");
    lv_obj_t *t4 = lv_tabview_add_tab(tv_main, "KART");

    lv_obj_set_style_pad_all(t1, 4, 0);
    lv_obj_set_style_pad_all(t2, 4, 0);
    lv_obj_set_style_pad_all(t3, 4, 0);
    lv_obj_set_style_pad_all(t4, 4, 0);

    /* 3. BOTÕES GRANDINHOS PARA ANDAR P'OS LADOS (52px x 36px) */
    // Botão Esquerdo (◄)
    lv_obj_t *b_prev = lv_button_create(screen);
    lv_obj_set_size(b_prev, 52, 36);
    lv_obj_set_pos(b_prev, 0, 204);
    lv_obj_set_style_bg_color(b_prev, COLOR_SURFACE_2, 0);
    lv_obj_set_style_border_color(b_prev, COLOR_ACCENT, 0);
    lv_obj_set_style_border_width(b_prev, 2, 0);
    lv_obj_set_style_radius(b_prev, 4, 0);
    lv_obj_add_event_cb(b_prev, tab_next_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lp = lv_label_create(b_prev);
    lv_label_set_text(lp, "◄");
    lv_obj_set_style_text_color(lp, COLOR_ACCENT, 0);
    lv_obj_set_style_text_font(lp, &lv_font_montserrat_20, 0);
    lv_obj_center(lp);

    // Botão Direito (►)
    lv_obj_t *b_next = lv_button_create(screen);
    lv_obj_set_size(b_next, 52, 36);
    lv_obj_set_pos(b_next, 268, 204);
    lv_obj_set_style_bg_color(b_next, COLOR_SURFACE_2, 0);
    lv_obj_set_style_border_color(b_next, COLOR_ACCENT, 0);
    lv_obj_set_style_border_width(b_next, 2, 0);
    lv_obj_set_style_radius(b_next, 4, 0);
    lv_obj_add_event_cb(b_next, tab_prev_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *ln = lv_label_create(b_next);
    lv_label_set_text(ln, "►");
    lv_obj_set_style_text_color(ln, COLOR_ACCENT, 0);
    lv_obj_set_style_text_font(ln, &lv_font_montserrat_20, 0);
    lv_obj_center(ln);

    lv_obj_move_foreground(b_prev);
    lv_obj_move_foreground(b_next);

    /* =====================================================================================
     * ABA 1: VISÃO GERAL (TENSÃO, CORRENTE, TEMPERATURA E SOC)
     * ===================================================================================== */
    // Card 1: Tensão do Pack
    lv_obj_t *c_volt = create_card(t1, 150, 84);
    lv_obj_align(c_volt, LV_ALIGN_TOP_LEFT, 2, 2);
    lv_obj_t *t_volt = lv_label_create(c_volt);
    lv_label_set_text(t_volt, "TENSAO PACK");
    lv_obj_set_style_text_color(t_volt, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(t_volt, &lv_font_montserrat_12, 0);
    lv_obj_align(t_volt, LV_ALIGN_TOP_LEFT, 2, 0);

    lbl_pack_v = lv_label_create(c_volt);
    lv_label_set_text(lbl_pack_v, "538.4 V");
    lv_obj_set_style_text_color(lbl_pack_v, COLOR_TEXT_MAIN, 0);
    lv_obj_set_style_text_font(lbl_pack_v, &lv_font_montserrat_24, 0);
    lv_obj_align(lbl_pack_v, LV_ALIGN_LEFT_MID, 2, 2);

    lbl_pack_p = lv_label_create(c_volt);
    lv_label_set_text(lbl_pack_p, "POT: 12.4 kW");
    lv_obj_set_style_text_color(lbl_pack_p, COLOR_ACCENT, 0);
    lv_obj_set_style_text_font(lbl_pack_p, &lv_font_montserrat_12, 0);
    lv_obj_align(lbl_pack_p, LV_ALIGN_BOTTOM_LEFT, 2, 0);

    // Card 2: Corrente ISA IVT
    lv_obj_t *c_curr = create_card(t1, 150, 84);
    lv_obj_align(c_curr, LV_ALIGN_TOP_RIGHT, -2, 2);
    lv_obj_t *t_curr = lv_label_create(c_curr);
    lv_label_set_text(t_curr, "CORRENTE (IVT)");
    lv_obj_set_style_text_color(t_curr, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(t_curr, &lv_font_montserrat_12, 0);
    lv_obj_align(t_curr, LV_ALIGN_TOP_LEFT, 2, 0);

    lbl_pack_i = lv_label_create(c_curr);
    lv_label_set_text(lbl_pack_i, "+24.2 A");
    lv_obj_set_style_text_color(lbl_pack_i, COLOR_OK, 0);
    lv_obj_set_style_text_font(lbl_pack_i, &lv_font_montserrat_24, 0);
    lv_obj_align(lbl_pack_i, LV_ALIGN_LEFT_MID, 2, 2);

    lv_obj_t *sub_curr = lv_label_create(c_curr);
    lv_label_set_text(sub_curr, "SHNT: 500A CAL");
    lv_obj_set_style_text_color(sub_curr, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(sub_curr, &lv_font_montserrat_12, 0);
    lv_obj_align(sub_curr, LV_ALIGN_BOTTOM_LEFT, 2, 0);

    // Card 3: Temperatura Máx / Mín
    lv_obj_t *c_temp = create_card(t1, 150, 86);
    lv_obj_align(c_temp, LV_ALIGN_BOTTOM_LEFT, 2, -2);
    lv_obj_t *t_temp = lv_label_create(c_temp);
    lv_label_set_text(t_temp, "TEMPERATURAS");
    lv_obj_set_style_text_color(t_temp, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(t_temp, &lv_font_montserrat_12, 0);
    lv_obj_align(t_temp, LV_ALIGN_TOP_LEFT, 2, 0);

    lbl_temp_range = lv_label_create(c_temp);
    lv_label_set_text(lbl_temp_range, "34.8 C");
    lv_obj_set_style_text_color(lbl_temp_range, COLOR_WARN, 0);
    lv_obj_set_style_text_font(lbl_temp_range, &lv_font_montserrat_20, 0);
    lv_obj_align(lbl_temp_range, LV_ALIGN_LEFT_MID, 2, 0);

    lv_obj_t *t_sub = lv_label_create(c_temp);
    lv_label_set_text(t_sub, "MIN: 23.4C | MCU: 38C");
    lv_obj_set_style_text_color(t_sub, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(t_sub, &lv_font_montserrat_12, 0);
    lv_obj_align(t_sub, LV_ALIGN_BOTTOM_LEFT, 2, 0);

    // Card 4: SOC & Barra de Carga
    lv_obj_t *c_soc = create_card(t1, 150, 86);
    lv_obj_align(c_soc, LV_ALIGN_BOTTOM_RIGHT, -2, -2);
    lv_obj_t *t_soc = lv_label_create(c_soc);
    lv_label_set_text(t_soc, "ESTADO CARGA (SOC)");
    lv_obj_set_style_text_color(t_soc, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(t_soc, &lv_font_montserrat_12, 0);
    lv_obj_align(t_soc, LV_ALIGN_TOP_LEFT, 2, 0);

    lbl_soc_text = lv_label_create(c_soc);
    lv_label_set_text(lbl_soc_text, "86.5 %");
    lv_obj_set_style_text_color(lbl_soc_text, COLOR_ACCENT, 0);
    lv_obj_set_style_text_font(lbl_soc_text, &lv_font_montserrat_20, 0);
    lv_obj_align(lbl_soc_text, LV_ALIGN_LEFT_MID, 2, -2);

    bar_soc = lv_bar_create(c_soc);
    lv_obj_set_size(bar_soc, 136, 12);
    lv_obj_align(bar_soc, LV_ALIGN_BOTTOM_MID, 0, -2);
    lv_bar_set_range(bar_soc, 0, 100);
    lv_bar_set_value(bar_soc, 86, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar_soc, COLOR_SURFACE_2, 0);
    lv_obj_set_style_bg_color(bar_soc, COLOR_ACCENT, LV_PART_INDICATOR);
    lv_obj_set_style_radius(bar_soc, 3, 0);
    lv_obj_set_style_radius(bar_soc, 3, LV_PART_INDICATOR);

    /* =====================================================================================
     * ABA 2: CÉLULAS & DELTA (MÁX, MÍN, DESBALANCEAMENTO E SEGMENTOS)
     * ===================================================================================== */
    // Card Célula Máx
    lv_obj_t *c_cmax = create_card(t2, 98, 65);
    lv_obj_align(c_cmax, LV_ALIGN_TOP_LEFT, 2, 2);
    lv_obj_t *l1 = lv_label_create(c_cmax);
    lv_label_set_text(l1, "CELULA MAX");
    lv_obj_set_style_text_color(l1, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(l1, &lv_font_montserrat_12, 0);
    lv_obj_align(l1, LV_ALIGN_TOP_LEFT, 0, 0);

    lbl_cell_max = lv_label_create(c_cmax);
    lv_label_set_text(lbl_cell_max, "4.172 V");
    lv_obj_set_style_text_color(lbl_cell_max, COLOR_OK, 0);
    lv_obj_set_style_text_font(lbl_cell_max, &lv_font_montserrat_20, 0);
    lv_obj_align(lbl_cell_max, LV_ALIGN_BOTTOM_LEFT, 0, -4);

    // Card Célula Mín
    lv_obj_t *c_cmin = create_card(t2, 98, 65);
    lv_obj_align(c_cmin, LV_ALIGN_TOP_MID, 0, 2);
    lv_obj_t *l2 = lv_label_create(c_cmin);
    lv_label_set_text(l2, "CELULA MIN");
    lv_obj_set_style_text_color(l2, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(l2, &lv_font_montserrat_12, 0);
    lv_obj_align(l2, LV_ALIGN_TOP_LEFT, 0, 0);

    lbl_cell_min = lv_label_create(c_cmin);
    lv_label_set_text(lbl_cell_min, "4.138 V");
    lv_obj_set_style_text_color(lbl_cell_min, COLOR_TEXT_MAIN, 0);
    lv_obj_set_style_text_font(lbl_cell_min, &lv_font_montserrat_20, 0);
    lv_obj_align(lbl_cell_min, LV_ALIGN_BOTTOM_LEFT, 0, -4);

    // Card Delta Imbalance
    lv_obj_t *c_cdel = create_card(t2, 98, 65);
    lv_obj_align(c_cdel, LV_ALIGN_TOP_RIGHT, -2, 2);
    lv_obj_t *l3 = lv_label_create(c_cdel);
    lv_label_set_text(l3, "DELTA (mV)");
    lv_obj_set_style_text_color(l3, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(l3, &lv_font_montserrat_12, 0);
    lv_obj_align(l3, LV_ALIGN_TOP_LEFT, 0, 0);

    lbl_cell_delta = lv_label_create(c_cdel);
    lv_label_set_text(lbl_cell_delta, "34 mV");
    lv_obj_set_style_text_color(lbl_cell_delta, COLOR_ACCENT, 0);
    lv_obj_set_style_text_font(lbl_cell_delta, &lv_font_montserrat_20, 0);
    lv_obj_align(lbl_cell_delta, LV_ALIGN_BOTTOM_LEFT, 0, -4);

    // Grelha de 6 Segmentos (S1 a S6)
    lv_obj_t *c_segs = create_card(t2, 304, 106);
    lv_obj_align(c_segs, LV_ALIGN_BOTTOM_MID, 0, -2);
    lv_obj_t *t_seg_title = lv_label_create(c_segs);
    lv_label_set_text(t_seg_title, "TENSAO DOS 6 SEGMENTOS (144s3p)");
    lv_obj_set_style_text_color(t_seg_title, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(t_seg_title, &lv_font_montserrat_12, 0);
    lv_obj_align(t_seg_title, LV_ALIGN_TOP_LEFT, 4, 0);

    for (int i = 0; i < 6; i++) {
        int row = i / 3;
        int col = i % 3;
        int x = 4 + col * 98;
        int y = 20 + row * 40;

        lv_obj_t *s_box = lv_obj_create(c_segs);
        lv_obj_set_size(s_box, 92, 34);
        lv_obj_set_pos(s_box, x, y);
        lv_obj_set_style_bg_color(s_box, COLOR_SURFACE_2, 0);
        lv_obj_set_style_border_width(s_box, 0, 0);
        lv_obj_set_style_radius(s_box, 4, 0);
        lv_obj_set_style_pad_all(s_box, 2, 0);
        lv_obj_clear_flag(s_box, LV_OBJ_FLAG_SCROLLABLE);

        char s_label_txt[12];
        snprintf(s_label_txt, sizeof(s_label_txt), "SEG %d", i + 1);
        lv_obj_t *s_name = lv_label_create(s_box);
        lv_label_set_text(s_name, s_label_txt);
        lv_obj_set_style_text_color(s_name, COLOR_TEXT_MUTED, 0);
        lv_obj_set_style_text_font(s_name, &lv_font_montserrat_12, 0);
        lv_obj_align(s_name, LV_ALIGN_TOP_LEFT, 2, 0);

        lbl_seg_v[i] = lv_label_create(s_box);
        char s_val_txt[12];
        snprintf(s_val_txt, sizeof(s_val_txt), "%.1f V", bms.seg_voltage[i]);
        lv_label_set_text(lbl_seg_v[i], s_val_txt);
        lv_obj_set_style_text_color(lbl_seg_v[i], COLOR_TEXT_MAIN, 0);
        lv_obj_set_style_text_font(lbl_seg_v[i], &lv_font_montserrat_14, 0);
        lv_obj_align(lbl_seg_v[i], LV_ALIGN_BOTTOM_LEFT, 2, 0);
    }

    /* =====================================================================================
     * ABA 3: SISTEMA & SEGURANÇA (SDC, AIRs, PRECHARGE E HARDWARE AMS)
     * ===================================================================================== */
    lv_obj_t *c_safety = create_card(t3, 304, 175);
    lv_obj_align(c_safety, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *ts_title = lv_label_create(c_safety);
    lv_label_set_text(ts_title, "CIRCUITO DE SEGURANCA E CONTACTORES");
    lv_obj_set_style_text_color(ts_title, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(ts_title, &lv_font_montserrat_12, 0);
    lv_obj_align(ts_title, LV_ALIGN_TOP_LEFT, 4, 2);

    lbl_sdc_status = lv_label_create(c_safety);
    lv_label_set_text(lbl_sdc_status, "SDC (SHUTDOWN LOOP): FECHADO (OK)");
    lv_obj_set_style_text_color(lbl_sdc_status, COLOR_OK, 0);
    lv_obj_set_style_text_font(lbl_sdc_status, &lv_font_montserrat_14, 0);
    lv_obj_align(lbl_sdc_status, LV_ALIGN_TOP_LEFT, 4, 26);

    lbl_airs_status = lv_label_create(c_safety);
    lv_label_set_text(lbl_airs_status, "CONTACTORES: AIR+ [ON]  |  AIR- [ON]");
    lv_obj_set_style_text_color(lbl_airs_status, COLOR_TEXT_MAIN, 0);
    lv_obj_set_style_text_font(lbl_airs_status, &lv_font_montserrat_14, 0);
    lv_obj_align(lbl_airs_status, LV_ALIGN_TOP_LEFT, 4, 52);

    lbl_precharge_status = lv_label_create(c_safety);
    lv_label_set_text(lbl_precharge_status, "PRECHARGE: CONCLUIDO (HV_ON)");
    lv_obj_set_style_text_color(lbl_precharge_status, COLOR_ACCENT, 0);
    lv_obj_set_style_text_font(lbl_precharge_status, &lv_font_montserrat_14, 0);
    lv_obj_align(lbl_precharge_status, LV_ALIGN_TOP_LEFT, 4, 78);

    lbl_mcu_info = lv_label_create(c_safety);
    lv_label_set_text(lbl_mcu_info, "AMS MASTER MCU: 38.5 C  |  VENTOINHA: 60%");
    lv_obj_set_style_text_color(lbl_mcu_info, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(lbl_mcu_info, &lv_font_montserrat_12, 0);
    lv_obj_align(lbl_mcu_info, LV_ALIGN_TOP_LEFT, 4, 110);

    lbl_slaves_info = lv_label_create(c_safety);
    lv_label_set_text(lbl_slaves_info, "SLAVES DETETADOS: 12 / 12  |  PEC: OK (SEM ERROS)");
    lv_obj_set_style_text_color(lbl_slaves_info, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(lbl_slaves_info, &lv_font_montserrat_12, 0);
    lv_obj_align(lbl_slaves_info, LV_ALIGN_TOP_LEFT, 4, 134);

    /* =====================================================================================
     * ABA 4: HANDKART / CARREGADOR (CARRO DE CARGA & CONTROLO)
     * ===================================================================================== */
    lv_obj_t *c_chg = create_card(t4, 304, 105);
    lv_obj_align(c_chg, LV_ALIGN_TOP_MID, 0, 2);

    lv_obj_t *tchg = lv_label_create(c_chg);
    lv_label_set_text(tchg, "SESSAO DE CARGA (HANDKART)");
    lv_obj_set_style_text_color(tchg, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(tchg, &lv_font_montserrat_12, 0);
    lv_obj_align(tchg, LV_ALIGN_TOP_LEFT, 4, 2);

    lbl_chg_status = lv_label_create(c_chg);
    lv_label_set_text(lbl_chg_status, "ESTADO: CARREGAMENTO ATIVO");
    lv_obj_set_style_text_color(lbl_chg_status, COLOR_CYAN, 0);
    lv_obj_set_style_text_font(lbl_chg_status, &lv_font_montserrat_14, 0);
    lv_obj_align(lbl_chg_status, LV_ALIGN_TOP_LEFT, 4, 24);

    lbl_chg_target = lv_label_create(c_chg);
    lv_label_set_text(lbl_chg_target, "ALVO: 588.0 V  |  LIMITE: 12.0 A");
    lv_obj_set_style_text_color(lbl_chg_target, COLOR_TEXT_MAIN, 0);
    lv_obj_set_style_text_font(lbl_chg_target, &lv_font_montserrat_12, 0);
    lv_obj_align(lbl_chg_target, LV_ALIGN_TOP_LEFT, 4, 48);

    lbl_chg_session = lv_label_create(c_chg);
    lv_label_set_text(lbl_chg_session, "ENERGIA: 2.45 kWh  |  TEMPO: 30m 40s");
    lv_obj_set_style_text_color(lbl_chg_session, COLOR_ACCENT, 0);
    lv_obj_set_style_text_font(lbl_chg_session, &lv_font_montserrat_12, 0);
    lv_obj_align(lbl_chg_session, LV_ALIGN_TOP_LEFT, 4, 72);

    // Botão de Controlo: Alternar Modo Demo / CAN Real
    btn_demo_toggle = lv_button_create(t4);
    lv_obj_set_size(btn_demo_toggle, 304, 55);
    lv_obj_align(btn_demo_toggle, LV_ALIGN_BOTTOM_MID, 0, -6);
    lv_obj_set_style_bg_color(btn_demo_toggle, COLOR_ACCENT, 0);
    lv_obj_set_style_radius(btn_demo_toggle, 6, 0);
    lv_obj_add_event_cb(btn_demo_toggle, demo_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lbl_demo_btn = lv_label_create(btn_demo_toggle);
    lv_label_set_text(lbl_demo_btn, "MODO DEMO: ATIVO (TOCAR P/ ALTERNAR)");
    lv_obj_set_style_text_color(lbl_demo_btn, lv_color_black(), 0);
    lv_obj_set_style_text_font(lbl_demo_btn, &lv_font_montserrat_12, 0);
    lv_obj_center(lbl_demo_btn);

}

/* =========================================================================================
 * ATUALIZAÇÃO PERIÓDICA DA INTERFACE COM AS TELEMETRIAS
 * ========================================================================================= */
void update_ui() {
    static char buf[64];

    // Atualiza cabeçalho (visível em todas as abas)
    lv_label_set_text(header_state_label, bms.master_state);
    if (strcmp(bms.master_state, "HV_ON") == 0) {
        lv_obj_set_style_text_color(header_state_label, COLOR_OK, 0);
    } else if (strcmp(bms.master_state, "PRECHARGE") == 0) {
        lv_obj_set_style_text_color(header_state_label, COLOR_WARN, 0);
    } else {
        lv_obj_set_style_text_color(header_state_label, COLOR_TEXT_MUTED, 0);
    }

    if (bms.demo_mode) {
        lv_label_set_text(header_can_label, "DEMO");
        lv_obj_set_style_bg_color(header_dot, COLOR_ACCENT, 0);
    } else {
        snprintf(buf, sizeof(buf), "CAN: %lu", (unsigned long)bms.can_rx_total);
        lv_label_set_text(header_can_label, buf);
        lv_obj_set_style_bg_color(header_dot, COLOR_OK, 0);
    }

    // Só atualiza os widgets da aba ATUALMENTE VISÍVEL para evitar re-layouts inúteis!
    uint32_t active = (tv_main != NULL) ? lv_tabview_get_tab_active(tv_main) : 0;

    switch (active) {
        case 0: // GERAL
            snprintf(buf, sizeof(buf), "%.1f V", bms.pack_voltage);
            lv_label_set_text(lbl_pack_v, buf);

            snprintf(buf, sizeof(buf), "POT: %.1f kW", bms.pack_power);
            lv_label_set_text(lbl_pack_p, buf);

            if (bms.pack_current < 0) {
                snprintf(buf, sizeof(buf), "%.1f A", bms.pack_current);
                lv_obj_set_style_text_color(lbl_pack_i, COLOR_CYAN, 0);
            } else {
                snprintf(buf, sizeof(buf), "+%.1f A", bms.pack_current);
                lv_obj_set_style_text_color(lbl_pack_i, (bms.pack_current > 60.0f) ? COLOR_WARN : COLOR_OK, 0);
            }
            lv_label_set_text(lbl_pack_i, buf);

            snprintf(buf, sizeof(buf), "MAX: %.1f C", bms.temp_max);
            lv_label_set_text(lbl_temp_range, buf);

            snprintf(buf, sizeof(buf), "%.1f %%", bms.soc);
            lv_label_set_text(lbl_soc_text, buf);
            lv_bar_set_value(bar_soc, (int32_t)bms.soc, LV_ANIM_OFF);
            break;

        case 1: // CELULAS
            snprintf(buf, sizeof(buf), "%.3f V", bms.cell_v_max);
            lv_label_set_text(lbl_cell_max, buf);

            snprintf(buf, sizeof(buf), "%.3f V", bms.cell_v_min);
            lv_label_set_text(lbl_cell_min, buf);

            snprintf(buf, sizeof(buf), "%.0f mV", bms.cell_v_delta_mv);
            lv_label_set_text(lbl_cell_delta, buf);
            if (bms.cell_v_delta_mv > 60.0f) {
                lv_obj_set_style_text_color(lbl_cell_delta, COLOR_FAULT, 0);
            } else {
                lv_obj_set_style_text_color(lbl_cell_delta, COLOR_ACCENT, 0);
            }

            for (int i = 0; i < 6; i++) {
                snprintf(buf, sizeof(buf), "%.1f V", bms.seg_voltage[i]);
                lv_label_set_text(lbl_seg_v[i], buf);
            }
            break;

        case 2: // SISTEMA
            if (bms.sdc_closed) {
                lv_label_set_text(lbl_sdc_status, "SDC (SHUTDOWN LOOP): FECHADO (OK)");
                lv_obj_set_style_text_color(lbl_sdc_status, COLOR_OK, 0);
            } else {
                lv_label_set_text(lbl_sdc_status, "SDC (SHUTDOWN LOOP): ABERTO (FAULT)");
                lv_obj_set_style_text_color(lbl_sdc_status, COLOR_FAULT, 0);
            }
            break;

        case 3: { // KART / CARREGADOR
            uint32_t mins = bms.session_seconds / 60;
            uint32_t secs = bms.session_seconds % 60;
            snprintf(buf, sizeof(buf), "ENERGIA: %.2f kWh  |  TEMPO: %lum %lus", bms.session_kwh, (unsigned long)mins, (unsigned long)secs);
            lv_label_set_text(lbl_chg_session, buf);
            break;
        }

    }
}
