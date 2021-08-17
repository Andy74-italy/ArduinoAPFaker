#include <WiFiNINA.h>

class APFakerStruct
{
  public:
    char shortName[20];   // name displayed for reference
    char ssid[20];        // your network SSID (name)
    char pass[20];        // your network password (use for WPA, or use as key for WEP)
    int keyIndex;         // your network key Index number (needed only for WEP)
    bool operator!=(APFakerStruct comp){
      return strcmp(ssid, comp.ssid) != 0 || strcmp(pass, comp.pass) != 0 || (keyIndex != comp.keyIndex);
    }
} currentAPFaker, emptyAPFaker = { "", "", "", -1 };

enum CODE_RETURN {
  CS_SUCCESS = 0,
  CS_ERROR_WIFI_STATUS,
  CS_WARNING_FIRMWARE_VERSION,
  CS_ERROR_AP_INITIALIZATION
};

int clientCount = 0;
String connectedIP = "";

/**********************************
 * In this array will be recorded the 
 * statistics to show in the status 
 * screen (look the totalRecords variable)
 */
int MetricsData[3] = {0, 0, 0};

int led = LED_BUILTIN;
int status = WL_IDLE_STATUS;
WiFiServer server(80);

IPAddress GetIPAddress() {
  return WiFi.localIP();
}

CODE_RETURN setupAPFaker() {
  pinMode(led, OUTPUT);      // set the LED pin mode

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    return CS_ERROR_WIFI_STATUS;
  }

  currentAPFaker = emptyAPFaker;

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    return CS_WARNING_FIRMWARE_VERSION;
  }

  return CS_SUCCESS;
}

CODE_RETURN InitializationAP(APFakerStruct apf) {
  // Create open network. Change this line if you want to create an WEP network:
  status = WiFi.beginAP(apf.ssid);
  if (status != WL_AP_LISTENING) {
    return CS_ERROR_AP_INITIALIZATION;
  }
  delay(10000);
  server.begin();

  currentAPFaker = apf;

  Serial.print("IP to navigate: ");
  Serial.println(GetIPAddress());

  return CS_SUCCESS;
}

void StopAP(){
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
      MetricsData[1]++; // increment the current connected Device
      MetricsData[2]++; // increment the total connected Device
      Serial.println("Device connected to AP");
    } else {
      // a device has disconnected from the AP, and we are back in listening mode
      MetricsData[1]--; // decrement the current connected Device
      Serial.println("Device disconnected from AP");
    }
  }
}

void ProvidePageToClient(WiFiClient client, String page){
  Serial.println("sending page " + page + " started");
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();

  // the content of the HTTP response follows the header:
  client.print(page);

  client.print("Click <form method=\"POST\" action=\"/L\">here</form> to login<br>");
  client.print("Click <a href=\"/R\">here</a> to register<br>");

  // The HTTP response ends with another blank line:
  client.println();

  Serial.println("sending page " + page + " completed");
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

void APFakerClientLoopManager(){
  WiFiClient client = server.available();   // listen for incoming clients
  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character
          
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            ProvidePageToClient(client, "LOGIN PAGE");
            break;
          }
          else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }        
        if (currentLine.endsWith("POST /L")) {
          ProvidePageToClient(client, "CAN'T LOGIN");
          break;
        }
        if (currentLine.endsWith("GET /R")) {
          ProvidePageToClient(client, "CAN'T REGISTER");
          break;
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}
