#include <Arduino.h>
#include <LittleFS.h>
#include "ESPAsyncWebServer.h"
#include "webHandler.h"
#include "MzOTAHtml.h"
#include "uiHandler.h"
#include "imageHandler.h"

extern ImageHandler ih;
extern UIHandler uh;
extern char debugLog[2048];

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
    request->send(200, "text/plain", debugLog);
  });
}


AsyncWebServer *WebHandler::getServer(void) {
  return webServer;
}