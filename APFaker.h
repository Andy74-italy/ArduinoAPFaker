#include <WiFiNINA.h>

struct APFakerStruct
{
  char ssid[20];        // your network SSID (name)
  char pass[20];        // your network password (use for WPA, or use as key for WEP)
  int keyIndex;         // your network key Index number (needed only for WEP)
} currentAPFaker;

enum CODE_RETURN {
  CS_SUCCESS = 0,
  CS_ERROR_WIFI_STATUS,
  CS_WARNING_FIRMWARE_VERSION,
  CS_ERROR_AP_INITIALIZATION
};

WiFiClient ConnectedClients[100];
int clientCount = 0;

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

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    return CS_WARNING_FIRMWARE_VERSION;
  }

  return CS_SUCCESS;
}

CODE_RETURN InitializationAP(APFakerStruct apf) {
  // Create open network. Change this line if you want to create an WEP network:
  status = WiFi.beginAP(apf.ssid, apf.pass);
  if (status != WL_AP_LISTENING) {
    // Serial.println("Creating access point failed");
    // don't continue
    // while (true);
    return CS_ERROR_AP_INITIALIZATION;
  }
  delay(10000);
  server.begin();

  return CS_SUCCESS;
}

WiFiClient CheckForNewClient() {
  // compare the previous status to the current status
  if (status != WiFi.status()) {
    // it has changed update the variable
    status = WiFi.status();

    if (status == WL_AP_CONNECTED) {
      // a device has connected to the AP
      // Serial.println("Device connected to AP");
      
    } else {
      // a device has disconnected from the AP, and we are back in listening mode
      // Serial.println("Device disconnected from AP");
    }
  }

  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    ConnectedClients[clientCount] = client;
    clientCount++;
    // Serial.println("new client");           // print a message out the serial port
    // close the connection:
    client.stop();
    // Serial.println("client disconnected");
    return client;
  }
  return NULL;
}

void APFakerClientLoopManager(){
  for (int i = 0; i < clientCount; i++)
  {
    String currentLine = "";                // make a String to hold incoming data from the client
    while (ConnectedClients[i].connected()) {            // loop while the client's connected
      if (ConnectedClients[i].available()) {             // if there's bytes to read from the client,
        char c = ConnectedClients[i].read();             // read a byte, then
        // Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character
  
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            ConnectedClients[i].println("HTTP/1.1 200 OK");
            ConnectedClients[i].println("Content-type:text/html");
            ConnectedClients[i].println();
  
            // the content of the HTTP response follows the header:
            ConnectedClients[i].print("Click <a href=\"/H\">here</a> turn the LED on<br>");
            ConnectedClients[i].print("Click <a href=\"/L\">here</a> turn the LED off<br>");
  
            int randomReading = analogRead(A1);
            ConnectedClients[i].print("Random reading from analog pin: ");
            ConnectedClients[i].print(randomReading);
  
            // The HTTP response ends with another blank line:
            ConnectedClients[i].println();
            // break out of the while loop:
            break;
          }
          else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
  
        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(led, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(led, LOW);                // GET /L turns the LED off
        }
      }
    }
  }
}
