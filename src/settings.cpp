#include <Arduino.h>
#include "nvs_flash.h"
#include "settings.h"
#include <Preferences.h>
#include "imageHandler.h"
#include <lvgl.h>
#include "lvgl_v8_port.h"

//#define myDEBUG
#include "myDebug.h"

extern ImageHandler ih;

Settings* Settings::instance = 0;

Settings *Settings::getInstance() {
  if (!instance)
  {
      instance = new Settings();
  }
  return instance;
}

Settings::Settings() {
  nvs_flash_init();
  prefs.begin(DFF_NS, false);
}


void Settings::init(void) {
  lvgl_port_lock(portMAX_DELAY);
  s.mode.value = prefs.getUInt("mode", 0);
  DEBUG_PRINTF("Settings: mode=%d\n",s.mode.value);
  lv_dropdown_set_selected(s.mode.uiPtr, s.mode.value);
  ih.setUIPanel(s.mode.value);

  s.duration.value = prefs.getUInt("duration", 10);
  lv_slider_set_value(s.duration.uiPtr, s.duration.value, LV_ANIM_OFF);
  char buf[30];
  sprintf(buf,"Anzeigedauer (%d Sek)", s.duration.value);
  lv_label_set_text(s.lblDuration.uiPtr, buf);

  s.brightness.value = prefs.getUInt("brightness", 100);
  lv_slider_set_value(s.brightness.uiPtr, s.brightness.value, LV_ANIM_OFF);
  sprintf(buf,"Helligkeit (%d)", s.brightness.value);
  lv_label_set_text(s.lblBrightness.uiPtr, buf);

  s.transition.value = prefs.getUInt("transition", 0);
  lv_dropdown_set_selected(s.transition.uiPtr, s.transition.value);

  s.fromHour.value = prefs.getUInt("fromHour", 0);
  lv_spinbox_set_value(s.fromHour.uiPtr, s.fromHour.value);

  s.fromMin.value = prefs.getUInt("fromMin", 0);
  lv_spinbox_set_value(s.fromMin.uiPtr, s.fromMin.value);

  s.toHour.value = prefs.getUInt("toHour", 0);
  lv_spinbox_set_value(s.toHour.uiPtr, s.toHour.value);

  s.toMin.value = prefs.getUInt("toMin", 0);
  lv_spinbox_set_value(s.toMin.uiPtr, s.toMin.value);

  lvgl_port_unlock();
}


void Settings::setUInt(const char *name, uint32_t value) {
  DEBUG_PRINTF("setUInt: name='%s', value=%d\n", name, value);
  prefs.putUInt(name, value);
}
