/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#include "../../../../inc/MarlinConfigPre.h"

#if HAS_TFT_LVGL_UI

#include "../../../../MarlinCore.h"
#include "draw_ui.h"

#include "../../../../module/temperature.h"
#include "../../../../gcode/queue.h"
#include "../../../../gcode/gcode.h"

#include "../../../../module/probe.h"

extern lv_group_t *g;
static lv_obj_t   *scr, *labelV, *buttonV, *zOffsetText;
static lv_obj_t   *labelExt1, *labelBed;

static uint8_t has_adjust_z = 0;
static float step_dist=0.01;

enum {
  ID_BLTOUCH_INIT = 1,
  ID_BLTOUCH_ZOFFSETPOS,
  ID_BLTOUCH_ZOFFSETNEG,
  ID_BLTOUCH_SAVE,
  ID_BLTOUCH_TEST,
  ID_BLTOUCH_STEPS,
  ID_BLTOUCH_RETURN
 };


static void event_handler(lv_obj_t * obj, lv_event_t event) {
  if (event != LV_EVENT_RELEASED) return;
  char baby_buf[30] = { 0 };
  char str_1[16];
  switch (obj->mks_obj_id) {
    case ID_BLTOUCH_INIT:
      bltouch_do_init();
      break;
    case ID_BLTOUCH_ZOFFSETPOS:
      sprintf_P(baby_buf, PSTR("M290 Z%s"), dtostrf(step_dist, 1, 3, str_1));
      gcode.process_subcommands_now_P(PSTR(baby_buf));
      has_adjust_z = 1;
      break;
    case ID_BLTOUCH_ZOFFSETNEG:
      sprintf_P(baby_buf, PSTR("M290 Z%s"), dtostrf(-step_dist, 1, 3, str_1));
      gcode.process_subcommands_now_P(PSTR(baby_buf));
      has_adjust_z = 1;
      break;
    case ID_BLTOUCH_SAVE:
      queue.inject_P(PSTR("M211 S1\nM500\nG28 X Y"));
      break;
    case ID_BLTOUCH_TEST:
      queue.inject_P(PSTR("G28\nG1 Z0"));
      break;
    case ID_BLTOUCH_STEPS:
      if (abs((int)(100 * step_dist)) == 1)
        step_dist = 0.05;
      else if (abs((int)(100 * step_dist)) == 5)
        step_dist = 0.1;
      else
        step_dist = 0.01;
      disp_step_dist();
      break;
    case ID_BLTOUCH_RETURN:
      lv_clear_bltouch_settings();
      lv_draw_return_ui();
      break;

  }
}

void lv_draw_bltouch_settings(void) {
  scr = lv_screen_create(BLTOUCH_UI);
  // Create image buttons
  lv_big_button_create(scr, "F:/bmp_Add.bin"   , machine_menu.BLTouchOffsetpos, INTERVAL_V,                       titleHeight, event_handler, ID_BLTOUCH_ZOFFSETPOS);
  lv_obj_t *buttonExt1      = lv_img_create(scr, nullptr);
  lv_img_set_src(buttonExt1    , "F:/bmp_ext1_state.bin");
  lv_obj_set_pos(buttonExt1    , 171, 50);

  lv_obj_t *buttonBedstate = lv_img_create(scr, nullptr);
  lv_img_set_src(buttonBedstate, "F:/bmp_bed_state.bin");
  lv_obj_set_pos(buttonBedstate, 266, 50);

    labelExt1      = lv_label_create(scr, 161, 115, nullptr);
    labelBed       = lv_label_create(scr, 256, 115, nullptr);

    lv_obj_align(labelExt1, buttonExt1    , LV_ALIGN_IN_BOTTOM_MID, 2, 20);
    lv_obj_align(labelBed , buttonBedstate, LV_ALIGN_IN_BOTTOM_MID, 2, 20);


  zOffsetText = lv_label_create(scr                                           , 170, 140, nullptr);


  lv_big_button_create(scr, "F:/bmp_Dec.bin"   , machine_menu.BLTouchOffsetneg, BTN_X_PIXEL * 3 + INTERVAL_V * 4, titleHeight, event_handler, ID_BLTOUCH_ZOFFSETNEG);

  buttonV = lv_imgbtn_create(scr               , nullptr                      , INTERVAL_V                      , BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_BLTOUCH_STEPS);
  labelV = lv_label_create_empty(buttonV);

  lv_big_button_create(scr, "F:/bmp_in.bin"    , machine_menu.BLTouchTest     , BTN_X_PIXEL     + INTERVAL_V * 2, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_BLTOUCH_TEST);
  lv_big_button_create(scr, "F:/bmp_set.bin"   , machine_menu.BLTouchSave     , BTN_X_PIXEL * 2 + INTERVAL_V * 3, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_BLTOUCH_SAVE);
  lv_big_button_create(scr, "F:/bmp_return.bin", common_menu.text_back        , BTN_X_PIXEL * 3 + INTERVAL_V * 4, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_BLTOUCH_RETURN);

//  lv_big_button_create(scr, "F:/bmp_speed0.bin", machine_menu.BLTouchInit, INTERVAL_V, titleHeight, event_handler, ID_BLTOUCH_INIT);

  disp_step_dist();
  disp_bltouch_z_offset_value();
}

void disp_step_dist() {
  if ((int)(100 * step_dist) == 1)
    lv_imgbtn_set_src_both(buttonV, "F:/bmp_baby_move0_01.bin");
  else if ((int)(100 * step_dist) == 5)
    lv_imgbtn_set_src_both(buttonV, "F:/bmp_baby_move0_05.bin");
  else if ((int)(100 * step_dist) == 10)
    lv_imgbtn_set_src_both(buttonV, "F:/bmp_baby_move0_1.bin");

  if (gCfgItems.multiple_language) {
    if ((int)(100 * step_dist) == 1) {
      lv_label_set_text(labelV, move_menu.step_001mm);
      lv_obj_align(labelV, buttonV, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
    }
    else if ((int)(100 * step_dist) == 5) {
      lv_label_set_text(labelV, move_menu.step_005mm);
      lv_obj_align(labelV, buttonV, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
    }
    else if ((int)(100 * step_dist) == 10) {
      lv_label_set_text(labelV, move_menu.step_01mm);
      lv_obj_align(labelV, buttonV, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
    }
  }
}

void disp_bltouch_z_offset_value() {
  char buf[20];
  char str_1[16];
  sprintf_P(buf, PSTR("offset Z: %s mm"), dtostrf(probe.offset.z, 1, 2, str_1) );
  lv_label_set_text(zOffsetText, buf);

  sprintf(public_buf_l, printing_menu.temp1, (int)thermalManager.temp_hotend[0].celsius, (int)thermalManager.temp_hotend[0].target);
  lv_label_set_text(labelExt1, public_buf_l);

  #if HAS_HEATED_BED
    sprintf(public_buf_l, printing_menu.bed_temp, (int)thermalManager.temp_bed.celsius, (int)thermalManager.temp_bed.target);
    lv_label_set_text(labelBed, public_buf_l);
  #endif

}

void bltouch_do_init() {
  queue.inject_P(PSTR("M851 Z0\nG28\nG1 Z0 F200\nM211 S0"));
}

void lv_clear_bltouch_settings() { 
  #if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable) lv_group_remove_all_objs(g);
  #endif
  lv_obj_del(scr);
}

#endif // HAS_TFT_LVGL_UI
