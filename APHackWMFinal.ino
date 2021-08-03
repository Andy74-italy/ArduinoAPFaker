#include "DisplayMgr.h"

/******************************************
 * This function will identify the button 
 * pressed TO BE IMPLEMENT */
ACTIONS Action(){
  return ACT_NONE;
}

void setup() {
  setupDisplay();
  switch (setupAPFaker()){
    case CS_SUCCESS:
      DrawDisplay(" - OK -", "WiFi init success!");
      break;
    case CS_ERROR_WIFI_STATUS:
      DrawDisplay(" - ERROR -", "WiFi init failed!");
      while(true);
      break;
    case CS_WARNING_FIRMWARE_VERSION:
      DrawDisplay(" - WARNING -", "Update Firmware!");
      break;
  }
  delay(3000);
}

void loop() {
  displayLoop(Action());
}
