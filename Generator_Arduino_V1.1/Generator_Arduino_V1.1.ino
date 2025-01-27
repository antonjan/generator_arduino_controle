#include <ESP8266WiFi.h>
#include <WiFiManager.h> // Install this library
#include <PubSubClient.h> // Install this library
#include <ESP8266WebServer.h> // Install this library

// MQTT settings
const char* mqtt_server = "iot.giga.co.za"; // Replace with your MQTT server address
const int mqtt_port = 1883;                   // Replace with your MQTT server port
const char* mqtt_user = "homeassistant";      // Replace with your MQTT username
const char* mqtt_password = "h0m3@ss1$tant";  // Replace with your MQTT password
const char* mqtt_topic = "generator/tx/0002/cmnd/POWER";     // Replace with your MQTT topic

WiFiClient espClient;
PubSubClient client(espClient);
ESP8266WebServer server(80); // Web server on port 80

// Pin definitions
const int relayPin = 4; // GPIO 4 (Relay)
const int genInputPin = 5; // GPIO 5 (Input for generator)

// State tracking
bool relayState = LOW;     // Current state of the relay

// State tracking for GPIO 5
bool lastGenState = 1;

void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
  pinMode(genInputPin, INPUT_PULLUP);
  //pinMode(3, INPUT);

  // Setup WiFi
  WiFiManager wifiManager;
  wifiManager.setTimeout(180); // Timeout for configuration portal (180 seconds)

  if (!wifiManager.autoConnect("ESP8266_AutoConnect")) {
    Serial.println("Failed to connect to Wi-Fi. Restarting...");
    ESP.restart();
  }
  Serial.println("Connected to Wi-Fi!");

  // Setup MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);
  connectToMQTT();

  // Setup Web Server
  setupWebServer();

  Serial.println("Web server started.");

}

void loop() {
  // Ensure MQTT connection
  if (!client.connected()) {
    connectToMQTT();
  }
  server.handleClient();  // Handles web server requests
  client.loop();

  // Monitor GPIO 5 state and send MQTT message if it changes
  bool currentGenState = digitalRead(5);
  Serial.println(currentGenState);
  if (currentGenState != lastGenState) {
    lastGenState = currentGenState; // Update the state
    if (currentGenState == 0) {
      client.publish(mqtt_topic, "GEN ON");
      Serial.println("Generator ON detected, message sent: GEN ON");
    } else {
      client.publish(mqtt_topic, "GEN OFF");
      Serial.println("Generator OFF detected, message sent: GEN OFF");
    }
  }
  delay(2000);
}

// Connect to the MQTT server
void connectToMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds...");
      delay(5000);
    }
  }
}

// Setup the web server routes
void setupWebServer() {
  server.on("/", []() {
    String html = "<html><head><title>Relay Control</title></head><body>";
    html += "<h1>ESP8266 Relay Control</h1>";
    html += "<p>Relay State: <span style='color:";
    html += (relayState == HIGH) ? "green'>ON</span>" : "red'>OFF</span>";
    html += "</p>";
    html += "<button onclick=\"location.href='/on'\">Turn ON</button>";
    html += "<button onclick=\"location.href='/off'\">Turn OFF</button>";
    html += "</body></html>";
    server.send(200, "text/html", html);
  });

  server.on("/on", []() {
    relayState = HIGH;
    digitalWrite(relayPin, HIGH);
    server.sendHeader("Location", "/"); // Redirect back to the main page
    server.send(303);
    Serial.println("Relay turned ON via Web");
  });

  server.on("/off", []() {
    relayState = LOW;
    digitalWrite(relayPin, LOW);
    server.sendHeader("Location", "/"); // Redirect back to the main page
    server.send(303);
    Serial.println("Relay turned OFF via Web");
  });

  server.begin();
}

// Handle incoming MQTT messages
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Message received: ");
  Serial.println(message);

  if (message == "ON") {
    digitalWrite(relayPin, HIGH);
    Serial.println("Relay ON");
  } else if (message == "OFF") {
    digitalWrite(relayPin, LOW);
    Serial.println("Relay OFF");
  } else if (message == "IP") {
    
    sendIPAddress();
    Serial.println(WiFi.localIP());
  }
}

void sendIPAddress() {
  if (WiFi.status() == WL_CONNECTED) {
    IPAddress ip = WiFi.localIP();
    char ipStr[16]; // Buffer to hold the IP string
    snprintf(ipStr, sizeof(ipStr), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]); // Convert IP to string
    client.publish(mqtt_topic, ipStr); // Publish the IP to the topic "device/ip"
    Serial.print("Published IP Address: ");
    Serial.println(ipStr);
  }
}
