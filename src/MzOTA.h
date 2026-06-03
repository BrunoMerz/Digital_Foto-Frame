
#ifndef MzOTA_h
#define MzOTA_h

#include "Arduino.h"
#include "stdlib_noniso.h"

#ifndef MZOTA_USE_ASYNC_WEBSERVER
  #define MZOTA_USE_ASYNC_WEBSERVER 0
#endif

#ifndef MZOTA_DEBUG
  #define MZOTA_DEBUG 0
#endif

#ifndef UPDATE_DEBUG
  #define UPDATE_DEBUG 0
#endif

#if MZOTA_DEBUG
  #define MZOTA_DEBUG_MSG(x) Serial.printf("%s %s", "[MzOTA] ", x)
#else
  #define MZOTA_DEBUG_MSG(x)
#endif

#if defined(ESP8266)
  #include <functional>
  #include "FS.h"
  #include "LittleFS.h"
  #include "Updater.h"
  #include "StreamString.h"
  #if MZOTA_USE_ASYNC_WEBSERVER == 1
    #include "ESPAsyncTCP.h"
    #include "ESPAsyncWebServer.h"
    #define MZOTA_WEBSERVER AsyncWebServer
  #else
    #include "ESP8266WiFi.h"
    #include "WiFiClient.h"
    #include "ESP8266WebServer.h"
    #define MZOTA_WEBSERVER ESP8266WebServer
  #endif
  #define HARDWARE "ESP8266"
#elif defined(ESP32)
  #include <functional>
  #include "FS.h"
  #include "Update.h"
  #include "StreamString.h"
  #if MZOTA_USE_ASYNC_WEBSERVER == 1
    #include "AsyncTCP.h"
    #include "ESPAsyncWebServer.h"
    #define MZOTA_WEBSERVER AsyncWebServer
  #else
    #include "WiFi.h"
    #include "WiFiClient.h"
    #include "WebServer.h"
    #define MZOTA_WEBSERVER WebServer
  #endif
  #define HARDWARE "ESP32"
#elif defined(TARGET_RP2040) || defined(TARGET_RP2350) || defined(PICO_RP2040) || defined(PICO_RP2350)
  #include <functional>
  #include "Arduino.h"
  #include "StreamString.h"
  #include "FS.h"
  #include "LittleFS.h"
  #include "Updater.h"
  #if MZOTA_USE_ASYNC_WEBSERVER == 1
    #include "RPAsyncTCP.h"
    #include "ESPAsyncWebServer.h"
    #define MZOTA_WEBSERVER AsyncWebServer
  #else
    #include "WiFiClient.h"
    #include "WiFiServer.h"
    #include "WebServer.h"
    #define MZOTA_WEBSERVER WebServer
  #endif

  #if defined(TARGET_RP2040) || defined(PICO_RP2040)
    #define HARDWARE              "RP2040"
  #elif defined(TARGET_RP2350) || defined(PICO_RP2350)
    #define HARDWARE              "RP2350"
  #endif
  extern uint8_t _FS_start;
  extern uint8_t _FS_end;
#endif

enum OTA_Mode {
    OTA_MODE_FIRMWARE = 0,
    OTA_MODE_FILESYSTEM = 1
};

class MzOTAClass{
  public:
    MzOTAClass();

    void begin(MZOTA_WEBSERVER *server, const char * username = "", const char * password = "");

    void setAuth(const char * username, const char * password);
    void clearAuth();
    void setAutoReboot(bool enable);
    void loop();

    void onStart(std::function<void()> callable);
    void onProgress(std::function<void(size_t current, size_t final)> callable);
    void onEnd(std::function<void(bool success)> callable);
    
  private:
    MZOTA_WEBSERVER *_server;

    bool _authenticate;
    String _username;
    String _password;

    bool _auto_reboot = true;
    bool _reboot = false;
    unsigned long _reboot_request_millis = 0;

    String _update_error_str = "";
    unsigned long _current_progress_size;

    std::function<void()> preUpdateCallback = NULL;
    std::function<void(size_t current, size_t final)> progressUpdateCallback = NULL;
    std::function<void(bool success)> postUpdateCallback = NULL;
};

extern MzOTAClass MzOTA;
#endif
