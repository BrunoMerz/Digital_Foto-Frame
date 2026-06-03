/**
   MyHandler.h
   @autor    Bruno Merz

   @version  1.0

*/

#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

typedef struct {
    lv_obj_t *img_old;
    lv_obj_t *img_new;
} fade_ctx_t;

class ImageHandler {
    public:
        ImageHandler(void);
        void init(void);
        bool getImageList(const String &phpUrl);
        bool downloadJpg(const String &url);
        bool decodeJpgToBuffer(void);
        void prepareLvglImage(void);
        void displayImage(void);
        void disableImage(void);
        void enableImage(void);
        bool isImageEnabled(void);
        void setUIPanel(uint32_t mode);
        static void anim_set_opa(void *obj, int32_t v);
        static void fade_anim_ready_cb(lv_anim_t *a);
        lv_obj_t *lv_img_fade_to(lv_obj_t *old_img, const void *new_src, uint32_t time_ms);

        void loop(void);
        //void scaleImgToWidth(lv_obj_t *img, int target_w);
        void showImageFit(lv_obj_t *img, const lv_img_dsc_t *src);

        // Callback
        static bool jpg_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap);
 
        // used in callback
        static uint16_t *imgBuf1;       // RGB565 Puffer in PSRAM
        static uint16_t *imgBuf2;       // RGB565 Puffer in PSRAM
        static uint16_t *imgBuf3;       // RGB565 Puffer in PSRAM
        static uint16_t *imgBuf4;       // RGB565 Puffer in PSRAM
        static uint16_t sw;
        static uint8_t imgNo;

    private:
        uint16_t currentImageIndex;
        unsigned long lastUpdate;

        JsonDocument doc;
        JsonArray images;

        uint8_t *jpgBuffer;     // JPG Puffer in PSRAM
        size_t jpgBufferSize;   // aktuelle göße des jpgBuffer
        
        size_t jpgSize;
        uint16_t jpg_width;
        uint16_t jpg_height;
        
        int16_t sh;
        int8_t scale = 1;

        lv_img_dsc_t img_dsc1;
        lv_img_dsc_t img_dsc2;
        lv_img_dsc_t img_dsc3;
        lv_img_dsc_t img_dsc4;

        lv_obj_t *img1;
        lv_obj_t *img2;
        lv_obj_t *img3;
        lv_obj_t *img4;

        lv_obj_t *cont1;
        lv_obj_t *cont2;
        lv_obj_t *cont3;
        lv_obj_t *cont4;

        lv_obj_t *img_view;
        lv_obj_t *img_full;
        
        bool enabled;
};


