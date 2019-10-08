/*************************************************************************
 * Title: Simple ESP-8266 blynk/yunshan wifi relay control
 * File: esp8266_yunshan_relay.ino
 * Author: James Eli
 * Date: 12/25/2016
 *
 * This program controls a Yunshan wifi relay module communicating through 
 * the onboard esp-8266-12e module. The module is controlled from the
 * internet via the Blynk cloud app. 
 * 
 * Notes:
 *  (1) Requires the following arduino libraries:
 *      ESP8266
 *      Blynk
 *  (2) Compiled with arduino ide 1.6.12
 *  (3) Uses three Blynk app widgets:
 *       V0: button configured as a switch.
 *       V1: led.
 *       V2: led.
 *************************************************************************
 * Change Log:
 *   12/25/2016: Initial release. JME
 *   12/31/2016: Added input pin status. JME
 *   01/15/2017: Added volatile. JME
 *************************************************************************/
#include ESP8266WiFi.h
#include BlynkSimpleEsp8266.h

// Esp8266 pins.
#define ESP8266_GPIO2    2 // Blue LED.
#define ESP8266_GPIO4    4 // Relay control.
#define ESP8266_GPIO5    5 // Optocoupler input.
#define LED_PIN          ESP8266_GPIO2
// Blynk app authentication code.
char auth[] = "***";
// Wifi SSID.
const char ssid[] = "***";
// Wifi password.
const char password[] = "***";    
// Flag for sync on re-connection.
bool isFirstConnect = true; 
volatile int relayState = LOW;    // Blynk app pushbutton status.
volatile int inputState = LOW;    // Input pin state.

void setup() {
  pinMode( ESP8266_GPIO4, OUTPUT );       // Relay control pin.
  pinMode( ESP8266_GPIO5, INPUT_PULLUP ); // Input pin.
  pinMode( LED_PIN, OUTPUT );             // ESP8266 module blue LED.
  digitalWrite( LED_PIN, LOW );           // Turn on LED.
  Blynk.begin( auth, ssid, password );    // Initiate Blynk conection.
  digitalWrite( LED_PIN, HIGH );          // Turn off LED.
}

// This function runs every time Blynk connection is established.
BLYNK_CONNECTED() {
  if ( isFirstConnect ) {
    Blynk.syncAll();
    isFirstConnect = false;
  }
}

// Sync input LED.
BLYNK_READ( V2 ) {
  CheckInput();
}

// Blynk app relay command.
BLYNK_WRITE( V0 ) {
  if ( param.asInt() != relayState ) {
    relayState = !relayState;                  // Toggle state.
    digitalWrite( ESP8266_GPIO4, relayState ); // Relay control pin.
    Blynk.virtualWrite( V1, relayState*255 );  // Set Blynk app LED.
  }
}

// Debounce input pin.
int DebouncePin( void ) {
  // Read input pin.
  if ( digitalRead( ESP8266_GPIO5 ) == HIGH ) {
    // Debounce input.
    delay( 25 );
    if ( digitalRead( ESP8266_GPIO5 ) == HIGH )
      return HIGH;
  }
  return LOW;
}

// Set LED based upon state of input pin.
void CheckInput( void ) {
  if ( DebouncePin() != inputState ) {
    Blynk.virtualWrite( V2, inputState*255 );
    inputState = !inputState;
  }
}

// Main program loop.
void loop() {
  Blynk.run();
  CheckInput();
  //yield(); //Updated: 3/8/2017
}
