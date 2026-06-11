/**
   MyTime.cpp
   @autor    Bruno Merz

   @version  1.0
   @created  11.10.2021

*/
#include <WiFi.h>

#define myDEBUG
#include "MyDebug.h"

#include "MyTime.h"
#include "esp_sntp.h"

// the short strings for each day or month must be exactly dt_SHORT_STR_LEN
#define dt_SHORT_STR_LEN  3 // the length of short strings
#define dt_MAX_STRING_LEN 9

#define LANG_DAYS "Tage"
#define LANG_HOURS "Stunden"
#define LANG_MINUTES "Minuten"
#define LANG_SECONDS "Sekunden"

static char buffer[dt_MAX_STRING_LEN+1];  // must be big enough for longest string and the terminating null

const char monthStr0[] PROGMEM = "";
const char monthStr1[] PROGMEM = "January";
const char monthStr2[] PROGMEM = "February";
const char monthStr3[] PROGMEM = "March";
const char monthStr4[] PROGMEM = "April";
const char monthStr5[] PROGMEM = "May";
const char monthStr6[] PROGMEM = "June";
const char monthStr7[] PROGMEM = "July";
const char monthStr8[] PROGMEM = "August";
const char monthStr9[] PROGMEM = "September";
const char monthStr10[] PROGMEM = "October";
const char monthStr11[] PROGMEM = "November";
const char monthStr12[] PROGMEM = "December";

const char * monthNames[] =
{
    monthStr0,monthStr1,monthStr2,monthStr3,monthStr4,monthStr5,monthStr6,
    monthStr7,monthStr8,monthStr9,monthStr10,monthStr11,monthStr12
};


const char monthShortNames_P[] PROGMEM = "ErrJanFebMarAprMayJunJulAugSepOctNovDec";

const char dayStr0[] PROGMEM = "Err";
const char dayStr1[] PROGMEM = "Sunday";
const char dayStr2[] PROGMEM = "Monday";
const char dayStr3[] PROGMEM = "Tuesday";
const char dayStr4[] PROGMEM = "Wednesday";
const char dayStr5[] PROGMEM = "Thursday";
const char dayStr6[] PROGMEM = "Friday";
const char dayStr7[] PROGMEM = "Saturday";


//const PROGMEM char * const PROGMEM def[] =
const char * dayNames[] =
{
   dayStr0,dayStr1,dayStr2,dayStr3,dayStr4,dayStr5,dayStr6,dayStr7
};


const char dayShortNames_P[] PROGMEM = "ErrSunMonTueWedThuFriSat";

/* functions to return date strings */

MyTime* MyTime::instance = 0;

MyTime *MyTime::getInstance() {
  if (!instance)
  {
      instance = new MyTime();
  }
  return instance;
}

MyTime::MyTime(void) {
  mytm.startTime=0;
  mytm.upTime=0;
  mytm.tm_loc=0;
  //mytm.tm_ntpserver = "de.pool.ntp.de";
  mytm.tm_ntpserver = "pool.ntp.org";
  mytm.tm_timezone = "CET-1CEST,M3.5.0,M10.5.0/3";
  mytm.tm_lon = 8.45;
  mytm.tm_lat = 48.06;
  isInitialized=false;
}

int MyTime::second() {
  return mytm.tm_sec;
}
int MyTime::second(time_t t) {
  struct tm *lt = localtime(&t);
  return lt->tm_sec;
}

int MyTime::minute() {
  return mytm.tm_min;
}

int MyTime::minute(time_t t) {
  struct tm *lt = localtime(&t);
  return lt->tm_min;
}
int MyTime::hour() {
   return mytm.tm_hour;
}

int MyTime::hour(time_t t) {
  struct tm *lt = localtime(&t);
  return lt->tm_hour;
}

int MyTime::day() {
  return mytm.tm_mday;
}

int MyTime::day(time_t t) {
  struct tm *lt = localtime(&t);
  return lt->tm_mday;
}

int MyTime::weekday() {
  return mytm.tm_wday;
}

int MyTime::weekday(time_t t) {
  struct tm *lt = localtime(&t);
  return lt->tm_wday+1;
}

int MyTime::month() {
  return mytm.tm_mon;
}

int MyTime::month(time_t t) {
  struct tm *lt = localtime(&t);
  return lt->tm_mon+1;
}

int MyTime::year() {
  return mytm.tm_year;
}

int MyTime::year(time_t t) {
  struct tm *lt = localtime(&t);
  return lt->tm_year+1900;
}

time_t MyTime::localTm(void) {
  return mytm.tm_loc;
}

char *MyTime::monthStr(uint8_t month) {
  const char *p = monthNames[month];
  strcpy_P(buffer, (PGM_P)p);
  return buffer;
}

char *MyTime::dayStr(uint8_t day) {
  const char *p = dayNames[day];
  strcpy_P(buffer, (PGM_P)p);
  return buffer;
}

time_t MyTime::toLocal(time_t utc) {
  return utc + mytm.tm_tzoffset * 60;
}

time_t MyTime::toUTC(time_t loc) {
  return loc - mytm.tm_tzoffset * 60;
}

void MyTime::confTime(void) {
  struct tm utc;
  struct tm local;
  struct tm timeinfo;

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Waiting for WiFi...");
    delay(500);
  }

  // This is where your time zone is set
  DEBUG_PRINTF("confTime: TZ=%s, NTP=%s\n",mytm.tm_timezone.c_str(), mytm.tm_ntpserver.c_str());

  configTzTime(mytm.tm_timezone.c_str(), mytm.tm_ntpserver.c_str());
  sntp_set_sync_interval(10800000);   // 3 hours

  // Wait for first time
  while (!getLocalTime(&timeinfo) ) {
    String txt="Waiting for time: ";
    txt +=".";
    Serial.println(txt);
    vTaskDelay(pdMS_TO_TICKS(500));
  }

  getTime();

  mytm.startTime=mytm.tm_loc;

  // get UTC-Zeit
  gmtime_r(&mytm.tm_loc, &utc);

  // get local time (Zeitzone + DST korrekt)
  localtime_r(&mytm.tm_loc, &local);

  // calculate Offset (in seconds)
  mytm.tm_tzoffset = (mktime(&local) - mktime(&utc));

  // log result
  isInitialized = true;
  //Serial.printf("Offset to UTC: %d seconds\n", mytm.tm_tzoffset );
}


time_t MyTime::getLT(void) {
  return mytm.tm_loc;
}

String MyTime::getDate(void) {
  char *buffer;
  buffer = ctime(&mytm.tm_loc);
  String timeStamp(buffer);
  
  return timeStamp;
}


/**
   returns current time hours, minutes, seconds and day
*/
void MyTime::getTime(void) {
  now = time(nullptr); // now = localtime (in sec seit 1.1.1970)

  // Jetzt Localtime in tm Struktur konvertieren
  lt = localtime(&now);

  mytm.tm_hour = lt->tm_hour;
  mytm.tm_min  = lt->tm_min;
  mytm.tm_sec  = lt->tm_sec;
  mytm.tm_mday = lt->tm_mday;
  mytm.tm_wday = lt->tm_wday+1;
  mytm.tm_mon  = lt->tm_mon+1;
  mytm.tm_year = lt->tm_year+1900;
  mytm.tm_isdst= lt->tm_isdst;
  mytm.tm_loc  = now;
  mytm.tm_utc  = mytm.tm_loc - mytm.tm_tzoffset;

  mytm.upTime = mytm.tm_loc - mytm.startTime;
  //Serial.printf("MyTime.getTime: utc=%ld, loc=%ld\n",mytm.tm_utc,mytm.tm_loc);
}


/**
   sets current time hours, minutes, seconds and day
*/
void MyTime::setTime(char *tms) {
  struct timeval tv;
  struct tm tm_act;

    strptime(tms, "%Y-%m-%dT%H:%M:%S", &tm_act);
    time_t ts = mktime(&tm_act);
    tv.tv_sec = ts;
    configTime(0, 0, NULL, NULL, NULL);
    settimeofday(&tv,NULL);
    
    //DEBUG_PRINTF("setTime: %s\n",tms);
    //DEBUG_PRINTLN(getDate());

}

void MyTime::convertSeconds(time_t totalSeconds, uint8_t &days, uint8_t &hours, uint8_t &minutes, uint8_t &seconds) {
  days = totalSeconds / 86400;        // Ganze Tage berechnen
  hours = (totalSeconds % 86400) / 3600; // Übrige Stunden berechnen
  minutes = (totalSeconds % 3600) / 60;  // Übrige Minuten berechnen
  seconds = totalSeconds % 60;        // Übrige Sekunden berechnen
}

String MyTime::convertSeconds(time_t totalSeconds, boolean shortText) {
  uint8_t days;
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
  convertSeconds(totalSeconds, days, hours, minutes, seconds);
  return(String(days)   + " " + (shortText?"d":LANG_DAYS)    + ", " +
        String(hours)   + " " + (shortText?"h":LANG_HOURS)   + ", " + 
        String(minutes) + " " + (shortText?"m":LANG_MINUTES) + ", " + 
        String(seconds) + " " + (shortText?"s":LANG_SECONDS));
}