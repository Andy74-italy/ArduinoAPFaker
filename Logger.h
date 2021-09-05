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
  "WARNING",
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
      String dtm = String(rtc.getYear()) + "/" + String(rtc.getMonth()) + "/" + String(rtc.getDay())
                 + " " + String(rtc.getHours()) + ":" + String(rtc.getMinutes()) + ":" + String(rtc.getSeconds())
                 + " - " + titleLog[type];
      Serial.println(dtm);            RecordLog(dtm, true);
      Serial.println(message);        RecordLog(message, true);
      Serial.println();               RecordLog("", true);
    }
};
