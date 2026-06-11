/**
   MyTime.h
   @autor    Bruno Merz

   @version  1.0
   @created  11.10.2021

   Handle time

*/

#pragma once

#include <Arduino.h>

#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval


typedef struct mytm
{
  uint8_t  tm_sec;
  uint8_t  tm_min;
  uint8_t  tm_hour;
  uint8_t  tm_mday;
  uint8_t  tm_mon;
  uint16_t tm_year;
  uint8_t  tm_wday;
  uint8_t  tm_yday;
  uint8_t  tm_isdst;
  int      tm_tzoffset;
  double   tm_lon;
  double   tm_lat;
  String   tm_timezone;
  String   tm_ntpserver;
  time_t   tm_utc;
  time_t   tm_loc;
  time_t   startTime;
  time_t   upTime;
} MYTM;

class MyTime {
  public:
    static MyTime* getInstance();
    void   getTime(void);
    time_t getLT(void);
    String getDate(void);
    void   confTime(void);
    void   setTime(char *tms);
    String convertSeconds(time_t totalSeconds, boolean shortText=false);
    void   convertSeconds(time_t totalSeconds, uint8_t &days, uint8_t &hours, uint8_t &minutes, uint8_t &seconds);
    
    int second(void);
    int second(time_t t);
    int minute(void);
    int minute(time_t t);
    int hour(void);
    int hour(time_t t);
    int day(void);
    int day(time_t t);
    int weekday(void);
    int weekday(time_t t);
    int month(void);
    int month(time_t t);
    int year(void);
    int year(time_t t);
    time_t localTm(void);
    char *monthStr(uint8_t month);
    char *dayStr(uint8_t day);
    time_t toLocal(time_t utc);
    time_t toUTC(time_t loc);
    MYTM   mytm;
    bool isInitialized;
    
  private:
    MyTime(void);
    static MyTime *instance;
    time_t now;
    struct tm *lt;
};
