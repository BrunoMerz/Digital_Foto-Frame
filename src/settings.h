/**
   settings.h
   @autor    Bruno Merz

   @version  1.0

*/

#pragma once

#include <Arduino.h>
#include "lvgl.h"
#include "Preferences.h"

#define DFF_NS "dff"  // Digital Foto Frame Namespace in NVS

typedef struct {
  uint32_t value;
  lv_obj_t *uiPtr;
} NV;


typedef struct {
    NV orientation;
    NV mode;
    NV duration;
    NV transition;
    NV lblDuration;
    NV lblBrightness;
    NV brightness;
    NV fromHour;
    NV fromMin;
    NV toHour;
    NV toMin;
} SETTINGS;

class Settings {
  public:
    static Settings* getInstance();
    void init(void);
    void setUInt(const char *name, uint32_t value);

    SETTINGS s;

  private:
    Settings(void);
    static Settings *instance;
    
    Preferences prefs;
};