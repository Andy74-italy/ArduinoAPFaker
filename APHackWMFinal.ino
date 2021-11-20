#include "Configurator.h"

uint8_t buttonPin = A1;
bool buttonPressed = false;

/******************************************
 * This function will identify the button 
 * pressed TO BE IMPLEMENT */
float temp;
bool b1, b2, b3;
ACTIONS Action(){
  temp = analogRead(buttonPin);
  Logger::WriteLog(LOG_DEBUG, "Checking for button pressure (temp): " + String(temp));

  // Values using USB
//  b1 = (temp > 300 && temp < 320);
//  b2 = (temp > 620 && temp < 640);
//  b3 = (temp > 1020 && temp < 1025);
  //
  // Values using Battery
//  b1 = (temp > 160 && temp < 190);
//  b2 = (temp > 340 && temp < 370);
//  b3 = (temp > 870 && temp < 900);

  b1 = (temp > 300 && temp < 320) || (temp > 160 && temp < 190);
  b2 = (temp > 620 && temp < 640) || (temp > 340 && temp < 370);
  b3 = (temp > 1020 && temp < 1025) || (temp > 870 && temp < 900);

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

void SetDefaultConfig(){
  SetLogLevel(LOG_INFO);
  scrollActive = false;
}

void setup() {
  rtc.begin();

  Serial.begin(9600);
  delay(3000);
  SDSetup();
  Logger::WriteLog(LOG_INFO, F("Serial port initialized succesfully!"));

  SetDefaultConfig();
  loadConfig();

  setupDisplay();
  switch (setupAPFaker()){
    case CS_SUCCESS:
      DrawDisplay(F("SUCCESS"), F("WiFi ready"));
      Logger::WriteLog(LOG_INFO, F("WiFi initialized succesfully!"));
      break;
    case CS_ERROR_WIFI_STATUS:
      DrawDisplay(F("ERROR"), F("WiFi fail"));
      Logger::WriteLog(LOG_ERROR, F("WiFi initialization failed!"));
      while(true);
      break;
    case CS_WARNING_FIRMWARE_VERSION:
      DrawDisplay(F("WARNING"), F("Update FW"));
      Logger::WriteLog(LOG_WARNING, F("WiFi initialized succesfully, but need to update the Firmware version!"));
      break;
  }
  delay(3000);
  UpdateDisplay();
}

void loop() {
  displayLoop(Action());
}
