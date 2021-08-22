#include "DisplayMgr.h"

int buttonPin = A1;
bool buttonPressed = false;

/******************************************
 * This function will identify the button 
 * pressed TO BE IMPLEMENT */
ACTIONS Action(){
  float temp = analogRead(buttonPin);

  // 300 - 320 = UP button
  // 620 - 640 = MIDDLE button
  // 1020 - 1025 = DOWN button
  bool b1 = (temp > 300 && temp < 320);
  bool b2 = (temp > 620 && temp < 640);
  bool b3 = (temp > 1020 && temp < 1025);

  // check if all the button are released
  // in that case unset the pressed button flag 
  // and return the NO ACTION
  if (!b1 && !b2 && !b3){
    delay(250);
    buttonPressed = false;
    return ACT_NONE;
  }
  // if there's one button pressed
  // return no action (before proceed with a new 
  // action need to release the oldest one)
  if (buttonPressed){
    return ACT_NONE;
  }
  // Can prooced to set the state of the pressed 
  // button and return the relative action to accomplish
  buttonPressed = true;
  return ((b1) ? ACT_UP : ((b2) ? ACT_CONFIRM : ACT_DOWN));
}

void setup() {
  // SetLogLevel(LOG_DEBUG);
  rtc.begin();

  SDSetup();  
  Serial.begin(9600);
  Logger::WriteLog(LOG_INFO, "Serial port initialized succesfully!");
  
  setupDisplay();
  switch (setupAPFaker()){
    case CS_SUCCESS:
      DrawDisplay("-OK-", "WiFi init success!");
      Logger::WriteLog(LOG_INFO, "WiFi initialized succesfully!");
      break;
    case CS_ERROR_WIFI_STATUS:
      DrawDisplay("-ERROR-", "WiFi init failed!");
      Logger::WriteLog(LOG_ERROR, "WiFi initialization failed!");
      while(true);
      break;
    case CS_WARNING_FIRMWARE_VERSION:
      DrawDisplay("-WARNING-", "Update FW!");
      Logger::WriteLog(LOG_WARNING, "WiFi initialized succesfully, but need to initialize update the Firmware version!");
      break;
  }
  delay(3000);
  UpdateDisplay();
}

void loop() {
  displayLoop(Action());
}
