#include <Arduino.h>
#include <WiFi.h>
#include <lvgl.h>
#include "lvgl_v8_port.h"
#include "uiHandler.h"
#include "imageHandler.h"
#include "settings.h"
#include "myDebug.h"

using namespace esp_panel::board;

extern const lv_font_t arial14;
extern ImageHandler ih;
extern Board *board;
static Settings *se = Settings::getInstance();

// CALLBACKS

// Anzeigedauer
static void duration_event_cb(lv_event_t *e) {
    char buf[30];
    se->s.duration.value = lv_slider_get_value(se->s.duration.uiPtr);
    se->setUInt("duration", se->s.duration.value);
    //DEBUG_PRINTF("Dauer: %d\n", se->s.duration.value);
    sprintf(buf,"Anzeigedauer (%d Sek)", se->s.duration.value);
    lv_label_set_text(se->s.lblDuration.uiPtr, buf);
}

// Helligkeit
static void brightness_event_cb(lv_event_t *e) {
    char buf[30];
    se->s.brightness.value = lv_slider_get_value(se->s.brightness.uiPtr);
    se->setUInt("brightness", se->s.brightness.value);
    //DEBUG_PRINTF("Helligkeit: %d\n", se->s.brightness.value);
    sprintf(buf,"Helligkeit (%d)", se->s.brightness.value);
    lv_label_set_text(se->s.lblBrightness.uiPtr, buf);
    auto backlight = board->getBacklight();
    backlight->setBrightness(se->s.brightness.value);
}

// Übergänge
static void transition_event_cb(lv_event_t *e) {
    se->s.transition.value = lv_dropdown_get_selected(se->s.transition.uiPtr);
    se->setUInt("transition", se->s.transition.value);
    DEBUG_PRINTF("Übergang: %s\n", se->s.transition.value);
}

// Anzahl Bilder
static void mode_event_cb(lv_event_t *e) {
    se->s.mode.value = lv_dropdown_get_selected(se->s.mode.uiPtr);
    DEBUG_PRINTF("Mode: %d\n", se->s.mode.value);
    ih.setUIPanel(se->s.mode.value);
}

// Ausrichtung
static void orientation_event_cb(lv_event_t *e) {
    char *endptr;
    lv_obj_t *dd = lv_event_get_target(e);
    char buf[32];
    lv_dropdown_get_selected_str(dd, buf, sizeof(buf));
    DEBUG_PRINTF("Auswahl: %s\n", buf);
}

// Start Display Images
static void start_event_cb(lv_event_t *e) {
    UIHandler *uh = (UIHandler*) lv_event_get_user_data(e);
    DEBUG_PRINTLN("Button gedrückt!");
    uh->disableUI();
    ih.enableImage();
    lv_indev_t *indev = lv_indev_get_act();
    if(indev) lv_indev_reset(indev, NULL);
}

// Reset Digital Foto Frame
static void reset_event_cb(lv_event_t *e) {
    ESP.restart();
}

UIHandler::UIHandler() {
    enabled = false;
}


boolean UIHandler::init(void) {
    createSettingsUI();
    return true;
}

void UIHandler::enableUI(void) {
    lv_obj_clear_flag(canvas, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(canvas);
    lv_group_focus_obj(canvas);
    enabled = true;
}

void UIHandler::disableUI(void) {
    enabled = false;
    lv_obj_add_flag(canvas, LV_OBJ_FLAG_HIDDEN);
}

bool UIHandler::isUIEnabled(void) {
    return enabled;
}

void UIHandler::createSettingsUI(void) {
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &arial14); // Hier wird deine Font gesetzt

    // canvas erstellen
    canvas = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(canvas);
    lv_obj_set_size(canvas, 800, 480);
    lv_obj_set_style_pad_top(canvas, 40, 0);
    lv_obj_center(canvas);
    lv_obj_clear_flag(canvas, LV_OBJ_FLAG_SCROLLABLE);
   
    
    // Duration Label
    lv_obj_t *label1 = lv_label_create(canvas);
    lv_label_set_text(label1, "Anzeigedauer (0 Sek)");
    lv_obj_align(label1, LV_ALIGN_TOP_LEFT, 10, 10);
    se->s.lblDuration.uiPtr = label1;

    // Duration
    lv_obj_t *duration = lv_slider_create(canvas);
    lv_obj_set_width(duration, 200);
    lv_obj_align(duration, LV_ALIGN_TOP_LEFT, 10, 40);
    lv_slider_set_range(duration, 1, 60);
    lv_slider_set_value(duration, 10, LV_ANIM_OFF);
    lv_obj_add_event_cb(duration, duration_event_cb, LV_EVENT_RELEASED, NULL);
    se->s.duration.uiPtr = duration;

    // Brightness Label
    lv_obj_t *lblBrightness = lv_label_create(canvas);
    lv_label_set_text(lblBrightness, "Helligkeit (0)");
    lv_obj_align(lblBrightness, LV_ALIGN_TOP_LEFT, 10, 180);
    se->s.lblBrightness.uiPtr = lblBrightness;

    // Brightness
    lv_obj_t *brightness = lv_slider_create(canvas);
    lv_obj_set_width(brightness, 200);
    lv_obj_align(brightness, LV_ALIGN_TOP_LEFT, 10, 210);
    lv_slider_set_range(brightness, 1, 100);
    lv_slider_set_value(brightness, 100, LV_ANIM_OFF);
    lv_obj_add_event_cb(brightness, brightness_event_cb, LV_EVENT_RELEASED, NULL);
    se->s.brightness.uiPtr = brightness;

    // Transition Dropdown
    lv_obj_t *label2 = lv_label_create(canvas);
    lv_obj_add_style(label2, &style, 0);
    lv_label_set_text(label2, "Übergang");
    lv_obj_align(label2, LV_ALIGN_TOP_LEFT, 10, 80);

    lv_obj_t *transition = lv_dropdown_create(canvas);
    lv_dropdown_set_options(transition,
        "Keine\n"
        "Fade\n"
        "Slide");
    lv_obj_align(transition, LV_ALIGN_TOP_LEFT, 10, 110);
    lv_obj_add_event_cb(transition, transition_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    se->s.transition.uiPtr = transition;

    lv_obj_t *btn = lv_btn_create(canvas);
    lv_obj_set_size(btn, 200, 40);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 10, 350);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Bilder anzeigen");
    lv_obj_center(label);

    lv_obj_add_event_cb(btn, start_event_cb, LV_EVENT_CLICKED, this);

    lv_obj_t *rstbtn = lv_btn_create(canvas);
    lv_obj_set_size(rstbtn, 200, 40);
    lv_obj_align(rstbtn, LV_ALIGN_TOP_LEFT, 580, 350);
    lv_obj_t *rst = lv_label_create(rstbtn);
    lv_label_set_text(rst, "Neustart");
    lv_obj_center(rst);

    lv_obj_add_event_cb(rstbtn, reset_event_cb, LV_EVENT_CLICKED, this);


    /* =======================
    🧩 Layout TAB
    ======================= */

    // Modus Dropdown
    lv_obj_t *label3 = lv_label_create(canvas);
    lv_label_set_text(label3, "Anzahl Bilder");
    lv_obj_align(label3, LV_ALIGN_TOP_LEFT, 270, 10);

    lv_obj_t *mode = lv_dropdown_create(canvas);
    lv_dropdown_set_options(mode,
        "Ein Bild\n"
        "Vier Bilder");
    lv_obj_align(mode, LV_ALIGN_TOP_LEFT, 270, 40);
    lv_obj_add_event_cb(mode, mode_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    se->s.mode.uiPtr = mode;

    // Orientierung Dropdown
    lv_obj_t *label4 = lv_label_create(canvas);
    lv_label_set_text(label4, "Orientierung");
    lv_obj_align(label4, LV_ALIGN_TOP_LEFT, 270, 80);

    lv_obj_t *orientation = lv_dropdown_create(canvas);
    lv_dropdown_set_options(orientation,
        "0 °\n"
        "90 °\n"
        "180 °\n"
        "270 °\n");
    lv_obj_add_event_cb(orientation, orientation_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_align(orientation, LV_ALIGN_TOP_LEFT, 270, 110);
    se->s.orientation.uiPtr = orientation;


    /* =======================
    🎨 Design TAB
    ======================= */

    // Farbauswahl via Slider (RGB einfach gehalten)
    lv_obj_t *label5 = lv_label_create(canvas);
    lv_label_set_text(label5, "Rahmenfarbe (Rot)");
    lv_obj_align(label5, LV_ALIGN_TOP_LEFT, 540, 10);

    lv_obj_t *frame = lv_slider_create(canvas);
    lv_slider_set_range(frame, 0, 255);
    lv_obj_set_width(frame, 200);
    lv_obj_align(frame, LV_ALIGN_TOP_LEFT, 540, 40);

    // Vorschau Box
    lv_obj_t *color_box = lv_obj_create(canvas);
    lv_obj_set_size(color_box, 100, 100);
    lv_obj_align(color_box, LV_ALIGN_TOP_LEFT, 540, 80);

    // Event: Farbe ändern
    lv_obj_add_event_cb(frame, [](lv_event_t *e){
        lv_obj_t *frame = lv_event_get_target(e);
        int r = lv_slider_get_value(frame);

        lv_obj_t *box = (lv_obj_t*)lv_event_get_user_data(e);
        lv_color_t color = lv_color_make(r, 0, 0);

        lv_obj_set_style_bg_color(box, color, 0);
    }, LV_EVENT_VALUE_CHANGED, color_box);

    // Display IP-Adresse
    lv_obj_t *ip = lv_label_create(canvas);
    lv_label_set_text(ip, WiFi.localIP().toString().c_str());
    lv_obj_align(ip, LV_ALIGN_TOP_LEFT, 10, 420);

    enabled = true;
}
