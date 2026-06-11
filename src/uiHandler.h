/**
   UIHandler.h
   @autor    Bruno Merz

   @version  1.0

*/

#pragma once

#include <Arduino.h>
#include <lvgl.h>


class UIHandler {
  public:
    UIHandler(void);
    bool init(void);
    
    void enableUI(void);
    void disableUI(void);
    bool isUIEnabled(void);
    void updateDateTime(void);
    void debug(String txt);

  private:
    void createSettingsUI(void);
    void createSpinbox(lv_obj_t *sb, lv_coord_t x_ofs, lv_coord_t y_ofs, int32_t max, lv_obj_t **uiPtr);
    bool enabled;
    lv_obj_t *canvas;
    static lv_obj_t *selectedSpin;
    lv_obj_t *dt = nullptr;
    lv_obj_t *dbg = nullptr;
};