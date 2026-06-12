/*
 * DIGITAL FOTO FRAME
 * 
 * Monitor Exceptions: pio device monitor -f esp32_exception_decoder
 */

#include <Arduino.h>
//#include "esp_system.h"
#include <LittleFS.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "lvgl_v8_port.h"

// Import required libraries
#include <LittleFS.h>

#include <WiFi.h>

#include <ESPAsyncWebServer.h>
#include <DNSServer.h>

#include "Settings.h"

//#define myDEBUG
#include "myDebug.h"


#include "fileHandler.h"
#include "webHandler.h"
#include "imageHandler.h"
#include "uiHandler.h"
#include "MzOTA.h"
#include "credentials.h"
#include "MyTime.h"


extern void debugSysEnv(void);

//////////////////////
// Configuration
//////////////////////


const byte DNS_PORT = 53;


using namespace esp_panel::drivers;
using namespace esp_panel::board;

Board *board = nullptr;
Backlight *backlight = nullptr;

ImageHandler ih;
FileHandler fh;
WebHandler wh;
UIHandler uh;

char debugLog[2048];

static Settings *se;
static MyTime *mt;

static uint32_t startup = 0;
static unsigned long ota_progress_millis = 0;
static int16_t OTAStarted = 0;

void ConnectToWiFi(const char *ssid, const char *password);

void backlightOnOff(bool onOff) {
    static uint8_t backlightState = 100;
    if(backlightState != onOff) {
        if(backlight) {
            if(onOff) {
                backlight->on();
                DEBUG_PRINTLN("Display on");
                uh.debug("Display on");
            } else {
                backlight->off();
                DEBUG_PRINTLN("Display off");
                uh.debug("Display off");
            }
            backlightState = onOff;
        }
    }
}

void onOTAStart() {
  // Log when OTA has started
  DEBUG_PRINTLN("OTA update started!");
  DEBUG_PRINTF("OTA Start auf Core %d\n",xPortGetCoreID());
  OTAStarted = 1;
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    DEBUG_PRINTF("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    DEBUG_PRINTLN("OTA update finished successfully!");
  } else {
    DEBUG_PRINTLN("There was an error during OTA update!");
  }
  OTAStarted = 3;
}

void setup()
{
    Serial.begin(115200);
    delay(500);

    DEBUG_PRINTF("Initializing board, tearing=%d\n", LVGL_PORT_AVOID_TEARING_MODE);

    
    fh.init();

    ConnectToWiFi(WIFI_SSID, WIFI_PASSWORD);

    wh.init();
    wh.start();

    // Initialize Date Time from NTP
    mt = MyTime::getInstance();
    mt->confTime();

    board = new Board();
    board->init();
    backlight = board->getBacklight();

#if LVGL_PORT_AVOID_TEARING_MODE
    auto lcd = board->getLCD();
    // When avoid tearing function is enabled, the frame buffer number should be set in the board driver
    lcd->configFrameBufferNumber(LVGL_PORT_DISP_BUFFER_NUM);
#if ESP_PANEL_DRIVERS_BUS_ENABLE_RGB && CONFIG_IDF_TARGET_ESP32S3
    auto lcd_bus = lcd->getBus();
    /**
     * As the anti-tearing feature typically consumes more PSRAM bandwidth, for the ESP32-S3, we need to utilize the
     * "bounce buffer" functionality to enhance the RGB data bandwidth.
     * This feature will consume `bounce_buffer_size * bytes_per_pixel * 2` of SRAM memory.
     */
    if (lcd_bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB) {
        static_cast<BusRGB *>(lcd_bus)->configRGB_BounceBufferSize(lcd->getFrameWidth() * 10);
    }
#endif
#endif
    assert(board->begin());

    DEBUG_PRINTLN("Initializing LVGL");
    assert(lvgl_port_init(board->getLCD(), board->getTouch()));


    delay(500);

    DEBUG_PRINTLN("IMG Handler init");
    ih.init();
    DEBUG_PRINTLN("UI Handler init");
    uh.init();
    
    DEBUG_PRINTF("Screen width: %d, height: %d\n", lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
    se = Settings::getInstance();
    se->init();
    
    // Starting DNS Server
    //dns.start(DNS_PORT, "*", WiFi.localIP());

    // MzOTA
    MzOTA.begin(wh.getServer());
    MzOTA.setAutoReboot(true);

    // MzOTA callbacks
    MzOTA.onStart(onOTAStart);
    MzOTA.onProgress(onOTAProgress);
    MzOTA.onEnd(onOTAEnd);

    delay(100);

    debugSysEnv();
     
    startup = millis();
}

bool turnLcdOn(
    int currentHour, int currentMinute,
    int onHour, int onMinute,
    int offHour, int offMinute)
{
    int current = currentHour * 60 + currentMinute;
    int start   = onHour * 60 + onMinute;
    int end     = offHour * 60 + offMinute;

    if(millis() - startup > 300000) { // 5 Minuten bleibt das display auf jeden Fall an
        // Zeitspanne innerhalb eines Tages
        if (start < end)
        {
            return (current >= start && current < end);
        }
        // Zeitspanne geht über Mitternacht
        else if (start > end)
        {
            return (current >= start || current < end);
        }
        // start == end -> 24 Stunden EIN
        else
        {
            return true;
        }
    } else
        return true;
}

//////////////////////
// Loop: Digital Foto Frame
//////////////////////
static uint32_t last = 0;
void loop()
{
    switch(OTAStarted)
    {
        case 0: // normal loops
            if(millis() - last > 1000)
            {
                last = millis();
                uh.updateDateTime();
                if (turnLcdOn(mt->mytm.tm_hour, mt->mytm.tm_min,
                    se->s.fromHour.value, se->s.fromMin.value,
                    se->s.toHour.value, se->s.toMin.value)) {
                    backlightOnOff(true);
                } else {
                    backlightOnOff(false);
                }
            }

            delay(100);
            ih.loop();
            break;

        case 1: // Just started
            DEBUG_PRINTF("Loop auf Core %d\n",xPortGetCoreID());
            backlightOnOff(false);
            lvgl_port_lock(portMAX_DELAY);
            OTAStarted = 2;
            break;

        case 2: // During OTA
            MzOTA.loop();
            break;

        case 3: // OTA ended
            backlightOnOff(true);
            lvgl_port_unlock();
            OTAStarted = 4;
            break;
 
        case 4:
            MzOTA.loop();
            break;
    }
}


/**************************************************
* ConnectToWiFi(const char *ssid, const char *password)
*/
void ConnectToWiFi(const char *ssid, const char *password)
{
    DEBUG_PRINTLN("main::ConnectToWiFi()");

    // prepare WiFi
    WiFi.disconnect(true);             // that no old information is stored  
    WiFi.mode(WIFI_OFF);               // switch WiFi off  
    delay(1000);                       // short wait to ensure WIFI_OFF  
    WiFi.persistent(false);            // avoid that WiFi-parameters will be stored in persistent memory  

    // Connect to Wi-Fi
    WiFi.mode(WIFI_STA);
    delay(1000);  
    WiFi.begin(ssid, password);

    // try 15 times, then do a reconnect if still not conected
    int i = 0;
    DEBUG_PRINTLN("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) 
    {
        DEBUG_PRINT(".");

        if ( i == 15 )
        {
            DEBUG_PRINT("_reconnect_");
            WiFi.reconnect();
        }

        if (i > 30)
            break;
        
        delay(500);
        i++;
    }

    // Print ESP Local IP Address
    DEBUG_PRINTLN(WiFi.localIP());
}


