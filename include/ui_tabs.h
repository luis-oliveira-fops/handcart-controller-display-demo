#ifndef UI_TABS_H
#define UI_TABS_H

#include <lvgl.h>
#include "bms_telemetry.h"
#include "ui_colors.h"

/* Elementos gráficos LVGL (extern, definidos em ui_tabs.cpp) */
extern lv_obj_t *tv_main;
extern lv_obj_t *header_state_label;
extern lv_obj_t *header_can_label;
extern lv_obj_t *header_dot;

// Tab 1: Geral
extern lv_obj_t *lbl_pack_v;
extern lv_obj_t *lbl_pack_p;
extern lv_obj_t *lbl_pack_i;
extern lv_obj_t *lbl_temp_range;
extern lv_obj_t *lbl_soc_text;
extern lv_obj_t *bar_soc;

// Tab 2: Células
extern lv_obj_t *lbl_cell_max;
extern lv_obj_t *lbl_cell_min;
extern lv_obj_t *lbl_cell_delta;
extern lv_obj_t *lbl_seg_v[6];

// Tab 3: Sistema
extern lv_obj_t *lbl_sdc_status;
extern lv_obj_t *lbl_airs_status;
extern lv_obj_t *lbl_precharge_status;
extern lv_obj_t *lbl_mcu_info;
extern lv_obj_t *lbl_slaves_info;

// Tab 4: Handkart / Carregador
extern lv_obj_t *lbl_chg_status;
extern lv_obj_t *lbl_chg_target;
extern lv_obj_t *lbl_chg_session;
extern lv_obj_t *btn_demo_toggle;
extern lv_obj_t *lbl_demo_btn;

/* Constrói a interface gráfica (LVGL 9 TABVIEW com slide horizontal) */
void build_ui(void);

/* Atualização periódica da interface com as telemetrias */
void update_ui(void);

#endif // UI_TABS_H
