#include "DisplayMgr.h"

String config_file_name = F("apfaker.cnf");

int indexOfLogLevel(String value){
  for(int search = LOG_TOTAL - 1; search >= 0; search--)
    if (titleLog[search] == value) return search;
  return -1;
}

void setConfig(String setting, String value){
  Logger::WriteLog(LOG_INFO, "Loading setting " + setting + " with value " + value);
  if (setting == F("LOG-LEVEL")){
    value.replace(F("LOG_"), F(""));
    if (indexOfLogLevel(value) != -1)
      SetLogLevel((LOG_TYPE)value.toInt());
  } else if (setting == F("SCROLL-ACTIVE")){
    value.toLowerCase();
    scrollActive = (value == F("true"));
  }
}

String splitString(String str, char sep, int index)
{
 int found = 0;
 int strIdx[] = { 0, -1 };
 int maxIdx = str.length() - 1;

 for (int i = 0; i <= maxIdx && found <= index; i++)
 {
    if (str.charAt(i) == sep || i == maxIdx)
    {
      found++;
      strIdx[0] = strIdx[1] + 1;
      strIdx[1] = (i == maxIdx) ? i+1 : i;
    }
 }
 return found > index ? str.substring(strIdx[0], strIdx[1]) : F("");
}

/**********************************************
 * Syntax of the config file is
 * <SETTING NAME>=<SETTING VALUE>
 */
void loadConfig(){
  File configuration = SD.open("config/" + config_file_name, FILE_READ);
  if (!configuration){
    Serial.println(F("ERROR: Cannot open configuration file"));
    return;
  }

  Logger::WriteLog(LOG_INFO, F("Loading settings."));
  String conf = F(""), setting = F(""), value = F("");
  char letter;
  while (configuration.available()) {
    letter = configuration.read();
    conf += letter;
    if (letter == '\n'){
      conf.trim();
      if (conf.indexOf('#') == -1 && conf.length() > 0){
        setting = splitString(conf, '=', 0);
        value = splitString(conf, '=', 1);
        setConfig(setting, value);
      }
      conf = F("");
      setting = F("");
      value = F("");
    }
    Logger::WriteLog(LOG_DEBUG, "conf: " + conf);
  }
  configuration.close();
  Logger::WriteLog(LOG_INFO, F("Setting completed."));
}
