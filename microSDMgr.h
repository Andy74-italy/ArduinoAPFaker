#include <RTCZero.h>
#include <SD.h>

#define SD_PIN 6

String log_file_name = F("");
String records_file_name = F("");
File logfile, records;

bool SDAvailable = false, filesOpen = false;

void SDSetup() {
  Serial.println(F("Try to configure the SD!"));
  if (!SD.begin(SD_PIN)){
    Serial.println(F("Something went wrong!"));
    return;
  }
  SDAvailable = true;
  Serial.println(F("SD Setup completed!"));
}

void StartSDActivity(){
  Serial.println(F("Start SD ACtivity"));
  if (!SDAvailable){
    Serial.println(F("ERROR: SD seems to be not available!!!"));
    return;
  }
  RTCZero rtc;
  String partialFileName;
  partialFileName = String(rtc.getHours()) + String(rtc.getMinutes()) + String(rtc.getSeconds());
  log_file_name = String(partialFileName) + ".log";
  records_file_name = String(partialFileName) + ".txt";
  logfile = SD.open("logs/" + log_file_name, FILE_WRITE);
  if (!logfile){
    Serial.println(F("ERROR: Cannot open log file"));
    return;
  }
  records = SD.open("logs/" + records_file_name, FILE_WRITE);
  if (!records){
    Serial.println(F("ERROR: Cannot open record file"));
    return;
  }
  filesOpen = true;
  Serial.println(F("SD ACtivity Started"));
}

void StopSDActivity(){
  if (!SDAvailable)
    return;
  logfile.close();
  records.close();
  filesOpen = false;
}

void RecordLog(String logline, bool newline){
  if (!SDAvailable || !filesOpen || !logfile)
    return;
  if (newline)
    logfile.println(logline);
  else
    logfile.print(logline);
  logfile.flush();
}

void RecordCredentials(String credential){
  if (!SDAvailable || !filesOpen || !records)
    return;
  records.println(credential);
  records.flush();
}
