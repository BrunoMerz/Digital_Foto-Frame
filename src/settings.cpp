#include <Arduino.h>
#include "nvs_flash.h"
#include "settings.h"
#include <Preferences.h>
#include "imageHandler.h"
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

}


void Settings::setUInt(const char *name, uint32_t value) {
  DEBUG_PRINTF("setUInt: name='%s', value=%d\n", name, value);
  prefs.putUInt(name, value);
}
