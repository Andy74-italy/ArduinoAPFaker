#include <RTCZero.h>
#include <SD.h>

#define SD_PIN 6

String log_file_name = "";
String records_fie_name = "";

File logfile, records;

bool SDAvailable = false, filesOpen = false;

void SDSetup() {
  if (!SD.begin(SD_PIN))
    return;
  SDAvailable = true;
}

void StartSDActivity(){
  if (!SDAvailable)
    return;
  RTCZero rtc;
  String partialFileName;
  partialFileName = String(rtc.getHours()) + String(rtc.getMinutes()) + String(rtc.getSeconds());
  log_file_name = String(partialFileName) + ".log";
  records_fie_name = String(partialFileName) + ".txt";
  logfile = SD.open(log_file_name, FILE_WRITE);
  records = SD.open(records_fie_name, FILE_WRITE);
  filesOpen = true;
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
