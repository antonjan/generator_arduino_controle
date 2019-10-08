#include <ESP8266WiFi.h>
 
// Esp8266 pinouts
#define ESP8266_GPIO2    2  // Blue LED.
#define ESP8266_GPIO4    4  // Relay control. 
#define ESP8266_GPIO5    5  // Optocoupler input.
#define LED_PIN          ESP8266_GPIO2
// WiFi Definitions.
const char ssid[] = "Home";
const char pswd[] = "Telkom.1";
WiFiServer server( 80 );
volatile int relayState = 0;      // Relay state.
 
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
         s += "<head><style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}";
         s += "button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;";
         s += "text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}";
         s += "button2 {background-color: #77878A;}</style></head>";
  if ( req.indexOf( "OPTIONS" ) != -1 ) {
    s += "Allows: GET, OPTIONS<br>";
    s += "<p><a href=\"open\"><button class=\"button\">Generator Start</button></a></p>";
    //s += "<a href=\"open\">Generator Start</a><br>";
    s += "<p><a href=\"close\"><button class=\"button2\">Generator Stop</button></a></p>";
    //s += "<a href=\"close\">Generator Stop</a><br>";
    s += "<a href=\"io\">Input Status</a><br>";
  } else if ( req.indexOf( "GET" ) != -1 ) {
    if ( req.indexOf( "open" ) != -1 ) {
      // relay on!
        s += "Generator on!<br>";
      //s += "<a href=\"close\">Relay Close</a><br>";
      //s += "<a href=\"open\">Relay Open</a><br>";
      //s += "<a href=\"io\">Input Status</a><br>";
      relayState = 1;
      digitalWrite( ESP8266_GPIO4, 1 ); // Relay control pin.
     
    } else if ( req.indexOf( "close" ) != -1 ) {
      // relay off!
      s += "Generator off!<br>";
      //s += "<a href=\"close\">Relay Close</a><br>";
      //s += "<a href=\"open\">Relay Open</a><br>";
      //s += "<a href=\"io\">Input Status</a><br>";
      relayState = 0;
      digitalWrite( ESP8266_GPIO4, 0 ); // Relay control pin.
     
    } else if ( req.indexOf( "relay" ) != -1 ) {
      if ( relayState == 0 ){
         //relay off!
        s += "Generator off!<br>";
        s += "<a href=\"close\">Start Generator</a><br>";
        s += "<a href=\"open\">Stop Open</a><br>";
        s += "<a href=\"io\">Generator Status</a><br>";
      }else
        // relay on!
        s += "Generator on!<br>";
        //s += "<a href=\"close\">Relay Close</a><br>";
        //s += "<a href=\"open\">Relay Open</a><br>";
        //s += "<a href=\"io\">Input Status</a><br>";
    } else if ( req.indexOf( "io" ) != -1 ) {
      if ( digitalRead( ESP8266_GPIO5 ) == 0 ){
        s += "input io is:Generator Stoped!<br>";
        //s += "<a href=\"close\">Relay Close</a><br>";
        //s += "<a href=\"open\">Relay Open</a><br>";
        //s += "<a href=\"io\">Input Status</a><br>";
      }else
        s += "input io is:Generator Started!<br>";
        //s += "<a href=\"close\">Relay Close</a><br>";
        //s += "<a href=\"open\">Relay Open</a><br>";
        //s += "<a href=\"io\">Input Status</a><br>";
    } else if ( req.indexOf( "MAC" ) != -1 ) {
      uint8_t mac[WL_MAC_ADDR_LENGTH];
      WiFi.softAPmacAddress( mac );
      String macID = String( mac[WL_MAC_ADDR_LENGTH - 5], HEX) + String( mac[WL_MAC_ADDR_LENGTH - 4], HEX) +
                     String( mac[WL_MAC_ADDR_LENGTH - 3], HEX) + String( mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                     String( mac[WL_MAC_ADDR_LENGTH - 1], HEX) + String( mac[WL_MAC_ADDR_LENGTH], HEX);
      macID.toUpperCase();
      s += "MAC address: " + macID +"<br>";
      // s += "<a href=\"close\">Relay Close</a><br>";
      //s += "<a href=\"open\">Relay Open</a><br>";
      //s += "<a href=\"io\">Input Status</a><br>";
    } else
      s += "Invalid Request.<br> Generator Menu: open/close/relay/io/MAC (use Generato Start,Stop or input Status<br>";
      //s += "<a href=\"open\">Start Generator</a><br>";
      s += "<p><a href=\"open\"><button class=\"button\">Start Generator</button></a></p>";
      //s += "<a href=\"close\">Stop Generator</a><br>";
      s += "<p><a href=\"close\"><button class=\"button\">Stop Generator</button></a></p>";
      //s += "<a href=\"io\">Generator Status</a><br>";
      s += "<p><a href=\"io\"><button class=\"button\">Generator Status</button></a></p>";
  } else 
    s = "HTTP/1.1 501 Not Implemented\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
          
  client.flush();
  s += "<a href=\"http://www.giga.co.za\">Giga Technology</a>";
  s += "</html>\n";
 
  // Send the response to the client.
  client.print( s );
  delay( 1 );
  Serial.println( "Client response sent." );
}
 
void loop() {
  // Check if a client has connected.
  WiFiClient client = server.available();
  if ( client ) 
    GetClient( client );
}
 
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
