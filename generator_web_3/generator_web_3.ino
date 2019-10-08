#include <ESP8266WiFi.h>
 
// Esp8266 pinouts
#define ESP8266_GPIO2    2  // Blue LED.
#define ESP8266_GPIO4    4  // Relay control. 
#define ESP8266_GPIO5    5  // Optocoupler input.
#define LED_PIN          ESP8266_GPIO2
// WiFi Definitions.
const char ssid[] = "Giga7";
const char pswd[] = "6409125086089";
WiFiServer server( 80 );
volatile int relayState = 0;      // Relay state.
// Variable to store the HTTP request
String header;
// Current time
unsigned long currentTime = millis();
// Previous time
// Auxiliar variables to store the current output state
String input5State = "off";
String output4State = "off";

// Assign output variables to GPIO pins
//const int output5 = 5;
  const int input5 = 5;
  const int output4 = 4;
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000; 
void setup() {
  initHardware();
  connectWiFi();
  server.begin();
  
}
 
void GetClient( WiFiClient client ) {
  // Read the first line of the request.
  String req = client.readStringUntil( '\r' );
  Serial.println( req );
  client.flush();
 
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
 
  if ( req.indexOf( "OPTIONS" ) != -1 ) {
    s += "Allows: GET, OPTIONS";
 
  } else if ( req.indexOf( "GET" ) != -1 ) {
    if ( req.indexOf( "open" ) != -1 ) {
      // relay on!
      s += "relay on!";
      relayState = 1;
      digitalWrite( ESP8266_GPIO4, 1 ); // Relay control pin.
     
    } else if ( req.indexOf( "close" ) != -1 ) {
      // relay off!
      s += "relay off!";
      relayState = 0;
      digitalWrite( ESP8266_GPIO4, 0 ); // Relay control pin.
     
    } else if ( req.indexOf( "relay" ) != -1 ) {
      if ( relayState == 0 )
        // relay off!
        s += "relay off!";
      else
        // relay on!
        s += "relay on!";
 
    } else if ( req.indexOf( "io" ) != -1 ) {
      if ( digitalRead( ESP8266_GPIO5 ) == 0 )
        s += "input io is:0!";
      else
        s += "input io is:1!";
     
    } else if ( req.indexOf( "MAC" ) != -1 ) {
      uint8_t mac[WL_MAC_ADDR_LENGTH];
      WiFi.softAPmacAddress( mac );
      String macID = String( mac[WL_MAC_ADDR_LENGTH - 5], HEX) + String( mac[WL_MAC_ADDR_LENGTH - 4], HEX) +
                     String( mac[WL_MAC_ADDR_LENGTH - 3], HEX) + String( mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                     String( mac[WL_MAC_ADDR_LENGTH - 1], HEX) + String( mac[WL_MAC_ADDR_LENGTH], HEX);
      macID.toUpperCase();
      s += "MAC address: " + macID;
 
    } else
      s += "Invalid Request.<br> Try: open/close/relay/io/MAC";
 
  } else 
    s = "HTTP/1.1 501 Not Implemented\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
          
  client.flush();
  s += "</html>\n";
 
  // Send the response to the client.
  client.print( s );
  delay( 1 );
  Serial.println( "Client response sent." );
}
 
//void loop() {
  // Check if a client has connected.
//  WiFiClient client = server.available();
//  if ( client ) 
//    GetClient( client );
//}
 
void connectWiFi() {
  byte ledStatus = LOW;
  Serial.println();
  Serial.println( "Connecting to: " + String( ssid ) );
  // Set WiFi mode to station (as opposed to AP or AP_STA).
  WiFi.mode( WIFI_STA );
 
  // WiFI.begin([ssid], [passkey]) initiates a WiFI connection.
  // to the stated [ssid], using the [passkey] as a WPA, WPA2, or WEP passphrase.
  WiFi.begin( ssid, pswd );
 
  while ( WiFi.status() != WL_CONNECTED ) {
    // Blink the LED.
    digitalWrite( LED_PIN, ledStatus ); // Write LED high/low.
    ledStatus = ( ledStatus == HIGH ) ? LOW : HIGH;
    delay( 100 );
  }
 
  Serial.println( "WiFi connected" );  
  Serial.println( "IP address: " );
  Serial.println( WiFi.localIP() );
}
 
void initHardware() {
  Serial.begin( 9600 );
  pinMode( ESP8266_GPIO4, OUTPUT );       // Relay control pin.
  pinMode( ESP8266_GPIO5, INPUT_PULLUP ); // Input pin.
  pinMode( LED_PIN, OUTPUT );             // ESP8266 module blue LED.
  digitalWrite( ESP8266_GPIO4, 0 );       // Set relay control pin low.
}
void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /5/status") ) {
              Serial.println("GPIO 5 status");
              input5State = digitalRead(input5);
              //digitalWrite(output5, HIGH);
            
            } else if (header.indexOf("GET /4/on") >= 0) {
              Serial.println("GPIO 4 on");
              output4State = "on";
              digitalWrite(output4, HIGH);
            } else if (header.indexOf("GET /4/off") >= 0) {
              Serial.println("GPIO 4 off");
              output4State = "off";
              digitalWrite(output4, LOW);
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP8266 Web Server</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 5  
            client.println("<p>GPIO 5 - State " + input5State + "</p>");
            // If the input5State is off, it displays the ON button       
            if (input5State=="off") {
              client.println("<p><a href=\"/5/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 4  
            client.println("<p>GPIO 4 - State " + output4State + "</p>");
            // If the output4State is off, it displays the ON button       
            if (output4State=="off") {
              client.println("<p><a href=\"/4/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
