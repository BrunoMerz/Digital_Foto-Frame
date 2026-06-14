#include <LittleFS.h>

#include "fileHandler.h"

//#define myDebug
#include "myDebug.h"


FileHandler::FileHandler()
{
    
}

bool FileHandler::init(void) 
{
    bool ret = true;
    if ( !LittleFS.begin() ) 
    {
        if ( !LittleFS.format() )
        {
            DEBUG_PRINTLN(F("Formatierung nicht möglich"));
        }
        else
        {
            DEBUG_PRINTLN(F("Formatierung erfolgreich"));
            if ( !LittleFS.begin() )
            {
                DEBUG_PRINTLN(F("LittleFS Mount trotzdem fehlgeschlagen"));
                ret = false;
            }
            else 
            {
                DEBUG_PRINTLN(F("LittleFS Dateisystems erfolgreich gemounted!")); 
            }
        }
    }  
    else
    {
        DEBUG_PRINTLN(F("LittleFS erfolgreich gemounted!"));
    }
  
    return ret;
}

