#include <Arduino.h>
#include <LittleFS.h>
#include "ESPAsyncWebServer.h"
#include "webHandler.h"
#include "MzOTAHtml.h"
#include "uiHandler.h"
#include "imageHandler.h"
#include "MyTime.h"
#include "settings.h"
#include <lvgl.h>
#include "lvgl_v8_port.h"

using namespace esp_panel::drivers;

extern Backlight *backlight;
extern ImageHandler ih;
extern UIHandler uh;
extern char debugLog[2048];
extern bool turnLcdOn(
    int currentHour, int currentMinute,
    int onHour, int onMinute,
    int offHour, int offMinute);

static MyTime *mt = MyTime::getInstance();
static Settings *se = Settings::getInstance();

WebHandler::WebHandler(void) 
{

}

bool WebHandler::init(void) 
{
    webServer = new AsyncWebServer(80);
    return true;
}

// Alle webRequests
void WebHandler::start(void)
{
    webRequests();
    webServer->begin();
}

// Alle webRequests
void WebHandler::webRequests()
{
  webServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", MZ_HTML, sizeof(MZ_HTML));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  webServer->on("/restart", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect("/");
    delay(200);
    ESP.restart();
  });

  webServer->on("/pictures", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect("/");
    delay(200);
    uh.disableUI();
    ih.enableImage();
  });

  webServer->on("/debug", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    strcat(debugLog,mt->getDate().c_str());

    if(turnLcdOn(mt->mytm.tm_hour, mt->mytm.tm_min,
                    se->s.fromHour.value, se->s.fromMin.value,
                    se->s.toHour.value, se->s.toMin.value))
      strcat(debugLog," sollte ein sein\n");
    else
      strcat(debugLog," sollte aus sein\n");
    strcat(debugLog,"Backlight value=");
    strcat(debugLog,String(backlight->getBrightness()).c_str());
    strcat(debugLog,"\n");
    request->send(200, "text/plain", debugLog);
  });
}


AsyncWebServer *WebHandler::getServer(void) {
  return webServer;
}