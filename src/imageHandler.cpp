#include <Arduino.h>
#include <TJpg_Decoder.h>
#include <ArduinoJson.h>
#include <lvgl.h>
#include "lvgl_v8_port.h"
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include "imageHandler.h"
#include "uiHandler.h"
#include "settings.h"
#include "myDebug.h"

extern UIHandler uh;

static Settings *se = Settings::getInstance();

uint16_t ImageHandler::sw = 0;
uint8_t ImageHandler::imgNo = 0;
uint16_t *ImageHandler::imgBuf1 = nullptr;
uint16_t *ImageHandler::imgBuf2 = nullptr;
uint16_t *ImageHandler::imgBuf3 = nullptr;
uint16_t *ImageHandler::imgBuf4 = nullptr;

#define  PHPURL "https://bmerz.de/Dpf/Dpf.php?token=mss"

// Callback (click on Image)
static void image_event_cb(lv_event_t *e) {
    DEBUG_PRINTLN("image_event_cb");
    ImageHandler *ih = (ImageHandler*) lv_event_get_user_data(e);
    ih->disableImage();
    uh.enableUI();
    lv_indev_t *indev = lv_indev_get_act();
    if(indev) lv_indev_reset(indev, NULL);
}


ImageHandler::ImageHandler() {
    img1 = nullptr;
    img2 = nullptr;
    img3 = nullptr;
    img4 = nullptr;
    cont1 = nullptr;
    cont2 = nullptr;
    cont3 = nullptr;
    cont4 = nullptr;
    
    jpgSize = 0;
    currentImageIndex = 0;
    lastUpdate = 0;
    enabled = false;
    jpgBufferSize=512000;
    jpgBuffer = (uint8_t*)heap_caps_malloc(jpgBufferSize, MALLOC_CAP_SPIRAM);
}


void ImageHandler::init() {
    getImageList(PHPURL);

    // EIN gemeinsamer Container für die Image-Ansicht
    img_view = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(img_view);
    lv_obj_set_size(img_view, LV_PCT(100), LV_PCT(100));
    lv_obj_clear_flag(img_view, LV_OBJ_FLAG_SCROLLABLE);

     // 1. Container (Viewport)
    cont1 = lv_obj_create(img_view);
    lv_obj_remove_style_all(cont1);
    lv_obj_set_size(cont1, 400, 240);
    lv_obj_align(cont1, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_clear_flag(cont1, LV_OBJ_FLAG_SCROLLABLE);
    

    // 1. Image
    img1 = lv_img_create(cont1);
    lv_obj_add_flag(img1, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img1, image_event_cb, LV_EVENT_CLICKED, this);

    // 2. Container (Viewport)
    cont2 = lv_obj_create(img_view);
    lv_obj_remove_style_all(cont2);
    lv_obj_set_size(cont2, 400, 240);
    lv_obj_align(cont2, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_clear_flag(cont2, LV_OBJ_FLAG_SCROLLABLE);
 
    // 2. Image
    img2 = lv_img_create(cont2);
    lv_obj_add_flag(img2, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img2, image_event_cb, LV_EVENT_CLICKED, this);

    // 3. Container (Viewport)
    cont3 = lv_obj_create(img_view);
    lv_obj_remove_style_all(cont3);
    lv_obj_set_size(cont3, 400, 240);
    lv_obj_align(cont3, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_clear_flag(cont3, LV_OBJ_FLAG_SCROLLABLE);
 
    // 3. Image
    img3 = lv_img_create(cont3);
    lv_obj_add_flag(img3, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img3, image_event_cb, LV_EVENT_CLICKED, this);

    // 4. Container (Viewport)
    cont4 = lv_obj_create(img_view);
    lv_obj_remove_style_all(cont4);
    lv_obj_set_size(cont4, 400, 240);
    lv_obj_align(cont4, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_clear_flag(cont4, LV_OBJ_FLAG_SCROLLABLE);
 
    // 4. Image
    img4 = lv_img_create(cont4);
    lv_obj_add_flag(img4, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img4, image_event_cb, LV_EVENT_CLICKED, this);


    // Full image, falls nur ein Bild angezeigt wird
    img_full = lv_img_create(img_view);
    lv_obj_remove_style_all(img_full);
    lv_obj_add_flag(img_full, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img_full, image_event_cb, LV_EVENT_CLICKED, this);
    lv_obj_clear_flag(img_full, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(img_full, 800, 480);
    lv_obj_center(img_full);
    lv_obj_add_flag(img_full, LV_OBJ_FLAG_HIDDEN);
 
}


//////////////////////
// Nächstes Bild-URL holen
//////////////////////

bool ImageHandler::getImageList(const String &phpUrl) {
      
    HTTPClient http;
    http.begin(phpUrl);
    int httpCode = http.GET();
    bool ret=true;

    if (httpCode == HTTP_CODE_OK) {
        String imageUrl = http.getString();
        DeserializationError error = deserializeJson(doc, imageUrl);
        if (error) {
          DEBUG_PRINT("JSON Fehler: ");
          DEBUG_PRINTLN(error.c_str());
          ret = false;
        }
    } else {
        DEBUG_PRINTLN("HTTP Error: " + String(httpCode));
        ret = false;
    }

    http.end();
    
    images = doc["images"];

    DEBUG_PRINTF("setup: Anzahl=%d\n",images.size());
    return ret;
}

//////////////////////
// Step 2: JPG herunterladen
//////////////////////

bool ImageHandler::downloadJpg(const String &url) {
    DEBUG_PRINTF("Free PSRAM: %d bytes\n", ESP.getFreePsram());

    DEBUG_PRINTF("Free PSRAM total: %d\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    DEBUG_PRINTF("Largest free PSRAM block: %d\n", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
    WiFiClientSecure client;
    client.setInsecure(); // unsicher, für Produktion Zertifikat nutzen

    HTTPClient http;
    http.begin(client, url);
    int code = http.GET();
    if (code != HTTP_CODE_OK) {
        DEBUG_PRINTLN("Download failed: " + String(code));
        http.end();
        return false;
    }
 
    jpgSize = http.getSize();
    if(jpgSize > jpgBufferSize) {
        if(jpgBuffer)
            jpgBuffer = (uint8_t*)heap_caps_realloc(jpgBuffer, jpgSize, MALLOC_CAP_SPIRAM);
        else
            jpgBuffer = (uint8_t*)heap_caps_malloc(jpgSize, MALLOC_CAP_SPIRAM);

        jpgBufferSize = jpgSize;
    }

    if (jpgBuffer == NULL) {
        DEBUG_PRINTF("PSRAM allocation for %d bytes failed!\n", jpgSize);
        http.end();
        return false;
    }
    DEBUG_PRINTF("got  %d bytes\n", jpgSize );
    WiFiClient *stream = http.getStreamPtr();
    size_t totalRead = 0;
    size_t bytesRead = 0;
    uint8_t tmp[512];

    while (http.connected() || stream->available()) {

        size_t available = stream->available();

        if (available) {
            int bytesRead = stream->readBytes(tmp, sizeof(tmp));
            memcpy(jpgBuffer+totalRead, tmp, bytesRead);
            //buffer.insert(buffer.end(), tmp, tmp + bytesRead);
            totalRead += bytesRead;
        }
        delay(1);
    }

    //for(int i=0;i<10;i++)
    //    DEBUG_PRINTF("%2x ", *(jpgBuffer+i));

    DEBUG_PRINTLN("\n-------------");
    DEBUG_PRINTF("totalRead=%d, jpgSize=%d\n", totalRead, jpgSize);

    if (TJpgDec.getJpgSize(&jpg_width, &jpg_height, jpgBuffer, totalRead) == JDR_OK) {
        DEBUG_PRINTF("Width: %d  Height: %d\n", jpg_width, jpg_height);
        sw = (jpg_width + scale - 1) / scale;
        sh = (jpg_height + scale - 1) / scale;
    } else {
        DEBUG_PRINTLN("Could not get image size");
        http.end();
        return false;
    }

    http.end();
    return true;
}

//////////////////////
// Step 3: JPG -> RGB565 Buffer
//////////////////////

bool ImageHandler::jpg_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    for (int row = 0; row < h; row++) {
        if(se->s.mode.value) {
            switch(imgNo) {
                case 0:
                    memcpy(imgBuf1 + (y + row) * sw + x, bitmap + row * w, w * 2);
                    break;
                case 1:
                    memcpy(imgBuf2 + (y + row) * sw + x, bitmap + row * w, w * 2);
                    break;
                case 2:
                    memcpy(imgBuf3 + (y + row) * sw + x, bitmap + row * w, w * 2);
                    break;
                case 3:
                    memcpy(imgBuf4 + (y + row) * sw + x, bitmap + row * w, w * 2);
                    break;
            }
        } else {
             memcpy(imgBuf1 + (y + row) * sw + x, bitmap + row * w, w * 2);
        }
    }
    return true;
}


bool ImageHandler::decodeJpgToBuffer(void) {
    if(se->s.mode.value) {
        switch (imgNo) {
            case 0:
                if(!imgBuf1)
                    imgBuf1 = (uint16_t*)heap_caps_malloc(sw * sh * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
                else
                    imgBuf1 = (uint16_t*)heap_caps_realloc(imgBuf1, sw * sh * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

                if (!imgBuf1) {
                    DEBUG_PRINTLN("PSRAM1 allocation for LVGL image failed!");
                    return false;
                }
                break;
            case 1:
                if(!imgBuf2)
                    imgBuf2 = (uint16_t*)heap_caps_malloc(sw * sh * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
                else
                    imgBuf2 = (uint16_t*)heap_caps_realloc(imgBuf2, sw * sh * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

                if (!imgBuf2) {
                    DEBUG_PRINTLN("PSRAM2 allocation for LVGL image failed!");
                    return false;
                }
                break;
            case 2:
                if(!imgBuf3)
                    imgBuf3 = (uint16_t*)heap_caps_malloc(sw * sh * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
                else
                    imgBuf3 = (uint16_t*)heap_caps_realloc(imgBuf3, sw * sh * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

                if (!imgBuf3) {
                    DEBUG_PRINTLN("PSRAM3 allocation for LVGL image failed!");
                    return false;
                }
                break;
            case 3:
                if(!imgBuf4)
                    imgBuf4 = (uint16_t*)heap_caps_malloc(sw * sh * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
                else
                    imgBuf4 = (uint16_t*)heap_caps_realloc(imgBuf4, sw * sh * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

                if (!imgBuf4) {
                    DEBUG_PRINTLN("PSRAM4 allocation for LVGL image failed!");
                    return false;
                }
                break;
        }
    } else {
        if(!imgBuf1)
            imgBuf1 = (uint16_t*)heap_caps_malloc(sw * sh * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        else
            imgBuf1 = (uint16_t*)heap_caps_realloc(imgBuf1, sw * sh * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

        if (!imgBuf1) {
            DEBUG_PRINTLN("PSRAM11 allocation for LVGL image failed!");
            return false;
        }
    }
    TJpgDec.setCallback(jpg_output);
    TJpgDec.setSwapBytes(false);
    TJpgDec.setJpgScale(scale);

    JRESULT res = TJpgDec.drawJpg(0, 0, jpgBuffer, jpgSize);  
    return true;
    return res == JDR_OK;
}


//////////////////////
// LVGL Image Descriptor vorbereiten
//////////////////////

void ImageHandler::prepareLvglImage(void) {
    if(se->s.mode.value) {
        switch(imgNo) {
            case 0:
                img_dsc1.header.always_zero = 0;
                img_dsc1.header.w = sw;
                img_dsc1.header.h = sh;
                img_dsc1.header.cf = LV_IMG_CF_TRUE_COLOR;
                img_dsc1.data_size = sw * sh * 2;
                img_dsc1.data = (const uint8_t*)imgBuf1;
                break;
            case 1:
                img_dsc2.header.always_zero = 0;
                img_dsc2.header.w = sw;
                img_dsc2.header.h = sh;
                img_dsc2.header.cf = LV_IMG_CF_TRUE_COLOR;
                img_dsc2.data_size = sw * sh * 2;
                img_dsc2.data = (const uint8_t*)imgBuf2;
                break;
            case 2:
                img_dsc3.header.always_zero = 0;
                img_dsc3.header.w = sw;
                img_dsc3.header.h = sh;
                img_dsc3.header.cf = LV_IMG_CF_TRUE_COLOR;
                img_dsc3.data_size = sw * sh * 2;
                img_dsc3.data = (const uint8_t*)imgBuf3;
                break;
            case 3:
                img_dsc4.header.always_zero = 0;
                img_dsc4.header.w = sw;
                img_dsc4.header.h = sh;
                img_dsc4.header.cf = LV_IMG_CF_TRUE_COLOR;
                img_dsc4.data_size = sw * sh * 2;
                img_dsc4.data = (const uint8_t*)imgBuf4;
                break;
        }
    } else {
        img_dsc1.header.always_zero = 0;
        img_dsc1.header.w = sw;
        img_dsc1.header.h = sh;
        img_dsc1.header.cf = LV_IMG_CF_TRUE_COLOR;
        img_dsc1.data_size = sw * sh * 2;
        img_dsc1.data = (const uint8_t*)imgBuf1;
    }
}


//////////////////////
// LVGL Image anzeigen
//////////////////////

void ImageHandler::displayImage(void) {
    DEBUG_PRINTF("displayImage: %d, #=%d, w=%d, h=%d, s=%d, w=%d, h=%d, s=%d, w=%d, h=%d, s=%d, w=%d, h=%d,s=%d\n",
        se->s.mode.value,
        imgNo,
        img_dsc1.header.w,
        img_dsc1.header.h,
        img_dsc1.data_size,
        img_dsc2.header.w,
        img_dsc2.header.h,
        img_dsc2.data_size,
        img_dsc3.header.w,
        img_dsc3.header.h,
        img_dsc3.data_size,
        img_dsc4.header.w,
        img_dsc4.header.h,
        img_dsc4.data_size
    );
    if(se->s.mode.value) {
        switch(imgNo) {
            case 0:
                showImageFit(img1, &img_dsc1);
                break;
            case 1:
                showImageFit(img2, &img_dsc2);
                break;
            case 2:
                showImageFit(img3, &img_dsc3);
                break;
            case 3:
                showImageFit(img4, &img_dsc4);
                break;
        }
    } else {
        lv_img_set_src(img_full, &img_dsc1);
        lv_obj_set_size(img_full, img_dsc1.header.w, img_dsc1.header.h);
        lv_obj_center(img_full);
    }
}


void ImageHandler::loop(void) {
    if(enabled) {
        if (millis() - lastUpdate > se->s.duration.value*1000) {
            DEBUG_PRINTF("Anzahl=%d\n", images.size());
            if (images.size() == 0) return;

            const char* url = images[currentImageIndex];

            // Lock the mutex due to the LVGL APIs are not thread-safe
            lvgl_port_lock(-1);
            if (downloadJpg(url)) {
                DEBUG_PRINTF("Bild %s anzeigen\n", url);
                if (decodeJpgToBuffer()) {
                    prepareLvglImage();
                    displayImage();
                    DEBUG_PRINTLN("Displayed new image");
                    if(se->s.mode.value) {
                        imgNo++;
                        if(imgNo>3)
                            imgNo=0;
                    }
                }
                DEBUG_PRINTLN("Bild angezeigt");
            }
            // Release the mutex
            lvgl_port_unlock();
            
            currentImageIndex++;
            if (currentImageIndex >= images.size()) {
                currentImageIndex = 0;
                getImageList(PHPURL);
            }

            lastUpdate = millis();
        }
    }
}


void ImageHandler::disableImage(void) {
    enabled = false;
    lv_obj_add_flag(img_view, LV_OBJ_FLAG_HIDDEN);
}


void ImageHandler::enableImage(void) {
    lv_obj_clear_flag(img_view, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(img_view);
    enabled = true;
}

bool ImageHandler::isImageEnabled(void) {
    return enabled;
}


void ImageHandler::showImageFit(lv_obj_t *img, const lv_img_dsc_t *src)
{
    lv_img_set_src(img, src);

    // 3. Originalgröße
    lv_img_header_t header;
    lv_img_decoder_get_info(src, &header);

    // 4. Zoom berechnen
    uint32_t zoom;
    if(header.w > header.h)
        zoom = (400 * 256) / header.w;
    else
        zoom = (200 * 256) / header.h;
    lv_img_set_zoom(img, zoom);
    // 5. WICHTIG: Image NICHT verkleinern!
    // → sonst wird es gecroppt
    lv_obj_set_size(img, header.w, header.h);
    // 6. Zentrieren (jetzt wirkt Zoom korrekt)
    lv_obj_center(img);
}


void ImageHandler::setUIPanel(uint32_t mode) {
    if(mode) {
        // mehrere Images
        // alle Container sichtbar machen
        lv_obj_clear_flag(cont1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(cont2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(cont3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(cont4, LV_OBJ_FLAG_HIDDEN);
        // gesamt Image unsichtbar
        lv_obj_add_flag(img_full, LV_OBJ_FLAG_HIDDEN);
    } else {
        // einzel Image
        // alle Container unsichtbar
        lv_obj_add_flag(cont1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(cont2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(cont3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(cont4, LV_OBJ_FLAG_HIDDEN);
        // gesamt Image sichtbar
        lv_obj_clear_flag(img_full, LV_OBJ_FLAG_HIDDEN);
    }
    se->setUInt("mode", se->s.mode.value);
}





/* -------------------------------------------------------
 * Opacity setzen
 * -----------------------------------------------------*/
void ImageHandler::anim_set_opa(void *obj, int32_t v)
{
    lv_obj_set_style_opa((lv_obj_t *)obj, v, 0);
}


/* -------------------------------------------------------
 * Nach Animation altes Bild löschen
 * -----------------------------------------------------*/
void ImageHandler::fade_anim_ready_cb(lv_anim_t *a)
{
    fade_ctx_t *ctx = (fade_ctx_t *)a->user_data;

    if(ctx->img_old) {
        lv_obj_del(ctx->img_old);
    }

    lv_mem_free(ctx);
}


/* -------------------------------------------------------
 * Bild weich überblenden
 *
 * old_img  = aktuelles sichtbares Bild
 * new_src  = neues Bild
 * time_ms  = Dauer
 *
 * Rückgabe:
 * neues img-Objekt
 * -----------------------------------------------------*/
lv_obj_t *ImageHandler::lv_img_fade_to(lv_obj_t *old_img,
                         const void *new_src,
                         uint32_t time_ms)
{
    lv_obj_t *parent = lv_obj_get_parent(old_img);

    /* Neues Bild erzeugen */
    lv_obj_t *new_img = lv_img_create(parent);

    lv_img_set_src(new_img, new_src);

    /* gleiche Position/Größe */
    lv_obj_set_pos(new_img,
                   lv_obj_get_x(old_img),
                   lv_obj_get_y(old_img));

    lv_obj_set_size(new_img,
                    lv_obj_get_width(old_img),
                    lv_obj_get_height(old_img));

    /* Start: unsichtbar */
    lv_obj_set_style_opa(new_img, LV_OPA_TRANSP, 0);

    /* sicherstellen dass neues Bild oben liegt */
    lv_obj_move_foreground(new_img);

    fade_ctx_t *ctx = (fade_ctx_t *)lv_mem_alloc(sizeof(fade_ctx_t));

    ctx->img_old = old_img;
    ctx->img_new = new_img;

    lv_anim_t a;

    /* -------------------------------------------------
     * Neues Bild einblenden
     * ------------------------------------------------*/
    lv_anim_init(&a);

    lv_anim_set_var(&a, new_img);
    lv_anim_set_exec_cb(&a, anim_set_opa);

    lv_anim_set_values(&a,
                       LV_OPA_TRANSP,
                       LV_OPA_COVER);

    lv_anim_set_time(&a, time_ms);

    lv_anim_start(&a);

    /* -------------------------------------------------
     * Altes Bild ausblenden
     * ------------------------------------------------*/
    lv_anim_init(&a);

    lv_anim_set_var(&a, old_img);
    lv_anim_set_exec_cb(&a, anim_set_opa);

    lv_anim_set_values(&a,
                       LV_OPA_COVER,
                       LV_OPA_TRANSP);

    lv_anim_set_time(&a, time_ms);

    lv_anim_set_user_data(&a, ctx);
    lv_anim_set_ready_cb(&a, fade_anim_ready_cb);

    lv_anim_start(&a);

    return new_img;
}