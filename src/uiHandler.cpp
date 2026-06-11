#include <Arduino.h>
#include <WiFi.h>
#include <lvgl.h>
#include "lvgl_v8_port.h"
#include "uiHandler.h"
#include "imageHandler.h"
#include "settings.h"
#include "MyTime.h"

//#define myDEBUG
#include "myDebug.h"

using namespace esp_panel::board;

extern const lv_font_t arial14;
extern ImageHandler ih;
extern Board *board;
static Settings *se = Settings::getInstance();
static MyTime *mt = MyTime::getInstance();

lv_obj_t *UIHandler::selectedSpin = nullptr;

void  UIHandler::createSpinbox(lv_obj_t *sb, lv_coord_t x_ofs, lv_coord_t y_ofs, int32_t max, lv_obj_t **uiPtr) {
    *uiPtr = sb;

    lv_obj_align(sb, LV_ALIGN_TOP_LEFT, x_ofs, y_ofs);
    lv_obj_set_size(sb, 40, 40);
    lv_spinbox_set_range(sb, 0, max);
    lv_spinbox_set_digit_format(sb, 2, 0);
    lv_obj_set_style_text_align(sb, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    lv_obj_set_style_text_color(sb, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_color(sb, lv_color_black(), LV_PART_CURSOR);
    lv_obj_set_style_bg_opa(sb, 0, LV_PART_CURSOR);
    lv_spinbox_set_rollover(sb, true);


    // Event: Select Spin
    lv_obj_add_event_cb(sb, [](lv_event_t *e){
        selectedSpin = lv_event_get_target(e);
    }, LV_EVENT_RELEASED, sb);

    // Event: Spin value changed
    lv_obj_add_event_cb(sb, [](lv_event_t *e){
        lv_obj_t *obj = lv_event_get_target(e);
        
        if(se->s.fromHour.uiPtr==obj) {
            se->s.fromHour.value = lv_spinbox_get_value(obj);
            se->setUInt("fromHour", se->s.fromHour.value);
        } else if(se->s.fromMin.uiPtr==obj) {
            se->s.fromMin.value = lv_spinbox_get_value(obj);
            se->setUInt("fromMin", se->s.fromMin.value);
        } else if(se->s.toHour.uiPtr==obj) {
            se->s.toHour.value = lv_spinbox_get_value(obj);
            se->setUInt("toHour", se->s.toHour.value);
        } else if(se->s.toMin.uiPtr==obj) {
            se->s.toMin.value = lv_spinbox_get_value(obj);
            se->setUInt("toMin", se->s.toMin.value);
        }
        
    }, LV_EVENT_VALUE_CHANGED, sb);
    
}

// CALLBACKS

// Anzeigedauer
static void duration_event_cb(lv_event_t *e) {
    char buf[30];
    se->s.duration.value = lv_slider_get_value(se->s.duration.uiPtr);
    se->setUInt("duration", se->s.duration.value);
    //DEBUG_PRINTF("Dauer: %d\n", se->s.duration.value);
    sprintf(buf,"Anzeigedauer (%d Sek)", se->s.duration.value);
    lvgl_port_lock(portMAX_DELAY);
    lv_label_set_text(se->s.lblDuration.uiPtr, buf);
    lvgl_port_unlock();
}


// Brightness Event 
static void brightness_event_cb(lv_event_t *e) {
    char buf[30];
    lvgl_port_lock(portMAX_DELAY);
    se->s.brightness.value = lv_slider_get_value(se->s.brightness.uiPtr);
    se->setUInt("brightness", se->s.brightness.value);
    //DEBUG_PRINTF("Helligkeit: %d\n", se->s.brightness.value);
    sprintf(buf,"Helligkeit (%d)", se->s.brightness.value);
    
    lv_label_set_text(se->s.lblBrightness.uiPtr, buf);
    auto backlight = board->getBacklight();
    backlight->setBrightness(se->s.brightness.value);
    lvgl_port_unlock();
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
    lvgl_port_lock(portMAX_DELAY);
    lv_dropdown_get_selected_str(dd, buf, sizeof(buf));
    lvgl_port_unlock();
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
    lvgl_port_lock(portMAX_DELAY);
    lv_obj_clear_flag(canvas, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(canvas);
    lv_group_focus_obj(canvas);
    lvgl_port_unlock();
    enabled = true;
}

void UIHandler::disableUI(void) {
    enabled = false;
    lvgl_port_lock(portMAX_DELAY);
    lv_obj_add_flag(canvas, LV_OBJ_FLAG_HIDDEN);
    lvgl_port_unlock();
}

bool UIHandler::isUIEnabled(void) {
    return enabled;
}

void UIHandler::createSettingsUI(void) {

    lvgl_port_lock(portMAX_DELAY);

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &arial14); // Hier wird deine Font gesetzt

    // canvas erstellen
    DEBUG_PRINTLN("Create canvas");
    canvas = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(canvas);
    lv_obj_set_size(canvas, 800, 480);
    lv_obj_set_style_pad_top(canvas, 40, 0);
    lv_obj_center(canvas);
    lv_obj_clear_flag(canvas, LV_OBJ_FLAG_SCROLLABLE);
   
    
    // Duration Label
    DEBUG_PRINTLN("Create Duration Label");
    lv_obj_t *label1 = lv_label_create(canvas);
    lv_label_set_text(label1, "Anzeigedauer (0 Sek)");
    lv_obj_align(label1, LV_ALIGN_TOP_LEFT, 20, 10);
    se->s.lblDuration.uiPtr = label1;

    // Duration Slider
    DEBUG_PRINTLN("Create Slider");
    lv_obj_t *duration = lv_slider_create(canvas);
    lv_obj_set_width(duration, 200);
    lv_obj_align(duration, LV_ALIGN_TOP_LEFT, 20, 40);
    lv_slider_set_range(duration, 1, 60);
    lv_slider_set_value(duration, 10, LV_ANIM_OFF);
    lv_obj_add_event_cb(duration, duration_event_cb, LV_EVENT_RELEASED, NULL);
    se->s.duration.uiPtr = duration;


    // Transition Label
    DEBUG_PRINTLN("Create transition label");
    lv_obj_t *label2 = lv_label_create(canvas);
    lv_obj_add_style(label2, &style, 0);
    lv_label_set_text(label2, "Übergang");
    lv_obj_align(label2, LV_ALIGN_TOP_LEFT, 20, 100);

    // Transition Dropdown
    DEBUG_PRINTLN("Create transition Dropdown");
    lv_obj_t *transition = lv_dropdown_create(canvas);
    lv_dropdown_set_options(transition,
        "Keine\n"
        "Fade\n"
        "Slide");
    lv_obj_align(transition, LV_ALIGN_TOP_LEFT, 20, 130);
    lv_obj_add_event_cb(transition, transition_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    se->s.transition.uiPtr = transition;


    // Brightness Label
    DEBUG_PRINTLN("Create Brightness Label");
    lv_obj_t *lblBrightness = lv_label_create(canvas);
    lv_label_set_text(lblBrightness, "Helligkeit (0)");
    lv_obj_align(lblBrightness, LV_ALIGN_TOP_LEFT, 20, 190);
    se->s.lblBrightness.uiPtr = lblBrightness;

    // Brightness Slider
    DEBUG_PRINTLN("Create Brightnbess Slider");
    lv_obj_t *brightness = lv_slider_create(canvas);
    lv_obj_set_width(brightness, 200);
    lv_obj_align(brightness, LV_ALIGN_TOP_LEFT, 20, 210);
    lv_slider_set_range(brightness, 1, 100);
    lv_slider_set_value(brightness, 100, LV_ANIM_OFF);
    lv_obj_add_event_cb(brightness, brightness_event_cb, LV_EVENT_RELEASED, NULL);
    se->s.brightness.uiPtr = brightness;



    // Show Images
    DEBUG_PRINTLN("Create Show image button");
    lv_obj_t *btn = lv_btn_create(canvas);
    lv_obj_set_size(btn, 200, 40);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 10, 350);

    DEBUG_PRINTLN("Create Show Image Label");
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Bilder anzeigen");
    lv_obj_center(label);

    DEBUG_PRINTLN("Add image button event");
    lv_obj_add_event_cb(btn, start_event_cb, LV_EVENT_RELEASED, this);


    // Number of Images Label
    DEBUG_PRINTLN("Create no of images label");
    lv_obj_t *label3 = lv_label_create(canvas);
    lv_label_set_text(label3, "Anzahl Bilder");
    lv_obj_align(label3, LV_ALIGN_TOP_LEFT, 270, 10);

    // Number of Images Dropdown
    DEBUG_PRINTLN("Create no of images dropdown");
    lv_obj_t *mode = lv_dropdown_create(canvas);
    lv_dropdown_set_options(mode,
        "Ein Bild\n"
        "Vier Bilder");
    lv_obj_align(mode, LV_ALIGN_TOP_LEFT, 270, 40);
    se->s.mode.uiPtr = mode;
    
    DEBUG_PRINTLN("Add no of images event handler");
    lv_obj_add_event_cb(mode, mode_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Orientation Label
    lv_obj_t *label4 = lv_label_create(canvas);
    lv_label_set_text(label4, "Orientierung");
    lv_obj_align(label4, LV_ALIGN_TOP_LEFT, 270, 100);

    // Orientation Dropdown
    lv_obj_t *orientation = lv_dropdown_create(canvas);
    lv_dropdown_set_options(orientation,
        "0 °\n"
        "90 °\n"
        "180 °\n"
        "270 °\n");
    lv_obj_add_event_cb(orientation, orientation_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_align(orientation, LV_ALIGN_TOP_LEFT, 270, 130);
    se->s.orientation.uiPtr = orientation;


    // Framecolor Label 
    lv_obj_t *label5 = lv_label_create(canvas);
    lv_label_set_text(label5, "Rahmenfarbe (Rot)");
    lv_obj_align(label5, LV_ALIGN_TOP_LEFT, 530, 10);

    // Framecolor Slider 
    lv_obj_t *frame = lv_slider_create(canvas);
    lv_slider_set_range(frame, 0, 255);
    lv_obj_set_width(frame, 200);
    lv_obj_align(frame, LV_ALIGN_TOP_LEFT, 530, 40);

    // Event: Change Framecolor
    lv_obj_add_event_cb(frame, [](lv_event_t *e){
        lvgl_port_lock(portMAX_DELAY);
        lv_obj_t *frame = lv_event_get_target(e);
        int r = lv_slider_get_value(frame);
        
        lv_color_t color = lv_color_make(r, 0, 0);
     
        lv_obj_set_style_bg_color(frame, color, LV_PART_MAIN);
        lv_obj_set_style_bg_color(frame, color, LV_PART_INDICATOR);
        lvgl_port_unlock();
    }, LV_EVENT_RELEASED, frame);

    // Display Time Label
    lv_obj_t *title = lv_label_create(canvas);
    lv_label_set_text(title, "Anzeigezeit");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 530, 100);
    
    // Display Time from Label
    lv_obj_t *label_start = lv_label_create(canvas);
    lv_label_set_text(label_start, "Von:");
    lv_obj_align(label_start, LV_ALIGN_TOP_LEFT, 530, 130);
    
    // Display Time from hour Spinbox
    lv_obj_t *sb_hour_from = lv_spinbox_create(canvas);
    createSpinbox(sb_hour_from, 570, 130, 23, &se->s.fromHour.uiPtr);
 
    // Display Time from minute Spinbox
    lv_obj_t *sb_min_from = lv_spinbox_create(canvas);
    createSpinbox(sb_min_from, 630, 130, 59, &se->s.fromMin.uiPtr);

    // Display Time to Label
    lv_obj_t *label_end = lv_label_create(canvas);
    lv_label_set_text(label_end, "Bis:");
    lv_obj_align(label_end, LV_ALIGN_TOP_LEFT, 530, 170);

    // Display Time to hour Spinbox
    lv_obj_t *sb_hour_to = lv_spinbox_create(canvas);
    createSpinbox(sb_hour_to, 570, 180, 23, &se->s.toHour.uiPtr);
    
    // Display Time to minute Spinbox
    lv_obj_t *sb_min_to = lv_spinbox_create(canvas);
    createSpinbox(sb_min_to, 630, 180, 59, &se->s.toMin.uiPtr);
 
    // + Button
    lv_obj_t *pbtn = lv_btn_create(canvas);
    lv_obj_set_size(pbtn, 40, 40);
    lv_obj_align(pbtn, LV_ALIGN_TOP_LEFT, 570, 230);
    lv_obj_t *plabel = lv_label_create(pbtn);

    lv_label_set_text(plabel, "+");
    lv_obj_center(plabel);

    lv_obj_add_event_cb(pbtn, [](lv_event_t *e){
        lv_obj_t *pbtn = lv_event_get_target(e);
        if(selectedSpin) {
            lvgl_port_lock(portMAX_DELAY);
            lv_spinbox_increment(selectedSpin);
            lvgl_port_unlock();
        }
    }, LV_EVENT_CLICKED, pbtn);

    // - Button
    lv_obj_t *mbtn = lv_btn_create(canvas);
    lv_obj_set_size(mbtn, 40, 40);
    lv_obj_align(mbtn, LV_ALIGN_TOP_LEFT, 630, 230);
    lv_obj_t *mlabel = lv_label_create(mbtn);
    lv_label_set_text(mlabel, "-");
    lv_obj_center(mlabel);
    lv_obj_add_event_cb(mbtn, [](lv_event_t *e){
        lv_obj_t *mbtn = lv_event_get_target(e);
        if(selectedSpin) {
            lvgl_port_lock(portMAX_DELAY);
            lv_spinbox_decrement(selectedSpin);
            lvgl_port_unlock();
        }
    }, LV_EVENT_CLICKED, mbtn);
 
    // Restart Button
    lv_obj_t *rstbtn = lv_btn_create(canvas);
    lv_obj_set_size(rstbtn, 200, 40);
    lv_obj_align(rstbtn, LV_ALIGN_TOP_LEFT, 580, 350);
    lv_obj_t *rst = lv_label_create(rstbtn);
    lv_label_set_text(rst, "Neustart");
    lv_obj_center(rst);

    lv_obj_add_event_cb(rstbtn, reset_event_cb, LV_EVENT_RELEASED, this);

    // Display IP-Adresse
    lv_obj_t *ip = lv_label_create(canvas);
    lv_label_set_text(ip, WiFi.localIP().toString().c_str());
    lv_obj_align(ip, LV_ALIGN_TOP_LEFT, 10, 420);

    // Display Date and Time
    dt = lv_label_create(canvas);
    lv_label_set_text(dt, mt->getDate().c_str());
    lv_obj_align(dt, LV_ALIGN_TOP_LEFT, 600, 420);

    // Debug line
    dbg = lv_label_create(canvas);
    lv_label_set_text(dbg, "started");
    lv_obj_align(dbg, LV_ALIGN_TOP_LEFT, 150, 420);

    DEBUG_PRINTLN("UI Created");
    lvgl_port_unlock();
    enabled = true;
}

void UIHandler::updateDateTime(void) {
    if(dt) {
        mt->getTime();
        if(lvgl_port_lock(1000)) {
            lv_label_set_text(dt, mt->getDate().c_str());
            lvgl_port_unlock();
        }
    }
}

void UIHandler::debug(String txt) {
    if(dbg) {
        if(lvgl_port_lock(1000)) {
            lv_label_set_text(dbg, txt.c_str());
            lvgl_port_unlock();
        }
    }
}