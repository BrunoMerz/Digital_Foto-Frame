/**
   UIHandler.h
   @autor    Bruno Merz

   @version  1.0

*/

#pragma once

#include <Arduino.h>


class UIHandler {
  public:
    UIHandler(void);
    bool init(void);
    
    void enableUI(void);
    void disableUI(void);
    bool isUIEnabled(void);


  private:
    void createSettingsUI(void);
    bool enabled;
    lv_obj_t *canvas;
};