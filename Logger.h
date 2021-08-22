#include "microSDMgr.h"

enum LOG_TYPE {
  LOG_ERROR = 0,
  LOG_INFO,
  LOG_WARNING,
  LOG_DEBUG,
  LOG_TOTAL
} LogLevel = LOG_WARNING;

String titleLog[LOG_TOTAL] = {
  "ERROR",
  "INFO",
  "DEBUG"
};

RTCZero rtc;

void SetLogLevel(LOG_TYPE logLevel){
  LogLevel = logLevel;
}

class Logger{
  public:
    static void WriteLog(LOG_TYPE type, String message){
      if (type > LogLevel)
        return;
      char dt[16];
      char tm[16];
      sprintf(dt, "%02d/%02d/%02d", rtc.getYear(),rtc.getMonth(),rtc.getDay());
      sprintf(tm, "%02d:%02d:%02d", rtc.getHours(),rtc.getMinutes(),rtc.getSeconds());
      Serial.print(dt);               RecordLog(String(dt), false);
      Serial.print(" ");              RecordLog(" ", false);
      Serial.print(tm);               RecordLog(String(tm), false);
      Serial.print(" - ");            RecordLog(" - ", false);
      Serial.print(titleLog[type]);   RecordLog(titleLog[type], false);
      Serial.println();               RecordLog("", true);
      Serial.println(message);        RecordLog(message, true);
      Serial.println();               RecordLog("", true);
    }
};
