#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <ESP8266WebServer.h>

// MQTT settings
const char* mqtt_server = "iot.giga.co.za"; // Replace with your MQTT server address
const int mqtt_port = 1883;
const char* mqtt_user = "homeassistant"; // Replace with your MQTT username
const char* mqtt_password = "h0m3@ss1$tant"; // Replace with your MQTT password
const char* mqtt_topic = "generator/tx/0002/cmnd/POWER"; // Replace with your MQTT topic
const char* mqtt_topic2 = "generator/tx/0002/SWITCH"; 
const char* mqtt_topic3 = "generator/tx/0002/RESULT";

WiFiClient espClient;
PubSubClient client(espClient);
ESP8266WebServer server(80);

// Pin definitions
const int relayPin = 4;  // GPIO 4 (Relay)
const int genInputPin = 5;  // GPIO 5 (Input for generator)

// State tracking
bool relayState = LOW; // Start with the relay off
bool genInputState = LOW; // Track the state of genInputPin
bool lastGenState = 1;

// Default login credentials
const String loginUsername = "Admin";
String loginPassword = "Admin";  // Default password
bool isLoggedIn = false;

// Session timeout settings (in milliseconds)
const unsigned long SESSION_TIMEOUT = 300000;  // 5 minutes
unsigned long lastActivityTime = 0;

void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
  pinMode(genInputPin, INPUT_PULLUP);

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
  bool currentGenState = digitalRead(genInputPin);
  if (currentGenState != lastGenState) {
    lastGenState = currentGenState; // Update the state
    if (currentGenState == LOW) {
      client.publish(mqtt_topic2, "{\"Switch1\":{\"Action\":\"ON\"}}");
    } else {
      client.publish(mqtt_topic2, "{\"Switch1\":{\"Action\":\"OFF\"}}");
    }
  }

  // Check for session timeout
  if (isLoggedIn && millis() - lastActivityTime > SESSION_TIMEOUT) {
    Serial.println("Session timed out. Logging out...");
    isLoggedIn = false;
    server.sendHeader("Location", "/");
    server.send(303);  // Redirect to login page
  }
  delay(1000);
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
  // Login page
  server.on("/", HTTP_GET, []() {
    if (isLoggedIn) {
      sendControlPage();
    } else {
      sendLoginPage();
    }
  });

  // Handle login form submission
  server.on("/login", HTTP_POST, []() {
    String username = server.arg("username");
    String password = server.arg("password");
    if (username == loginUsername && password == loginPassword) {
      isLoggedIn = true;
      lastActivityTime = millis();  // Reset activity timer
      server.sendHeader("Location", "/");
      server.send(303);  // Redirect to the main page
    } else {
      server.send(401, "text/html", "<h1>Authentication Failed</h1><p>Invalid credentials, try again.</p>");
    }
  });

  // Toggle relay state
  server.on("/toggle", []() {
    relayState = !relayState; // Toggle the relay state
    digitalWrite(relayPin, relayState ? HIGH : LOW); // Turn on or off the relay

    // Send the appropriate MQTT message
    if (relayState == HIGH) {
      client.publish(mqtt_topic2, "{\"POWER\":\"ON\"}");
      Serial.println("Generator ON detected, message sent: ON");
    } else {
      client.publish(mqtt_topic2, "{\"POWER\":\"OFF\"}");
      Serial.println("Generator OFF detected, message sent: OFF");
    }

    server.sendHeader("Location", "/"); // Redirect back to the main page
    server.send(303); // HTTP redirect
    Serial.println(relayState == HIGH ? "Relay turned ON via Web" : "Relay turned OFF via Web");
  });

  server.begin();
}

void sendLoginPage() {
  String html = "<html><head><title>Login - Giga Technology</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; background-color: #f4f4f9; margin: 0; padding: 0; text-align: center; }";
  html += "h1 { color: #333; margin-top: 50px; }";
  html += "form { background-color: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1); display: inline-block; margin-top: 30px; }";
  html += "input[type='text'], input[type='password'] { width: 250px; padding: 10px; margin: 10px 0; border-radius: 5px; border: 1px solid #ccc; font-size: 16px; }";
  html += "input[type='submit'] { width: 100%; padding: 10px; background-color: #4CAF50; color: white; border: none; border-radius: 5px; font-size: 18px; cursor: pointer; }";
  html += "input[type='submit']:hover { background-color: #45a049; }";
  html += ".error { color: red; margin-top: 20px; }";
  html += "</style>";
  html += "</head><body>";
  html += "<h1>Login to Giga Technology</h1>";
  html += "<form action='/login' method='POST'>";
  html += "Username: <input type='text' name='username' required><br>";
  html += "Password: <input type='password' name='password' required><br>";
  html += "<input type='submit' value='Login'>";
  html += "</form>";
  if (!isLoggedIn) {
    html += "<div class='error'>Authentication Failed. Please check your credentials.</div>";
  }
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void sendControlPage() {
  String html = "<html><head><title>Giga Technology - Control</title>";
  html += "<meta http-equiv='refresh' content='5'>";  // Auto-refresh the page every 5 seconds
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; background-color: #f4f4f9; margin: 0; padding: 0; text-align: center; }";
  html += "h1 { color: #333; margin-top: 50px; }";
  html += "p { font-size: 18px; color: #555; }";
  html += ".status { font-size: 20px; font-weight: bold; }";
  html += ".status-on { color: green; }";
  html += ".status-off { color: red; }";
  html += "button { padding: 10px 20px; background-color: #4CAF50; color: white; border: none; border-radius: 5px; font-size: 16px; cursor: pointer; margin: 10px; }";
  html += "button.red { background-color: red; }"; // Button red for OFF state
  html += "button.green { background-color: green; }"; // Button green for ON state
  html += "button:hover { background-color: #45a049; }";
  html += "</style>";
  html += "</head><body>";
  html += "<h1><a href='https://www.giga.co.za/ocart/index.php?route=common/home' style='color: red;'>Giga Technology</a></h1>";
  html += "<p>Version: V1.2 27/01/2025</p>";

  // Display power status based on genInputPin state
  genInputState = digitalRead(genInputPin); // Read the state of genInputPin (GPIO 5)
  html += "<p>Power Status: <span class='status " + String(genInputState == LOW ? "status-on" : "status-off") + "'>" +
          (genInputState == LOW ? "ON" : "OFF") + "</span></p>";

  // Merge the ON and OFF buttons into one button with dynamic text and color based on relay state
  String buttonClass = (relayState == HIGH) ? "green" : "red"; // Choose class based on relay state
  String buttonText = (relayState == HIGH) ? "Generator ON" : "Generator OFF"; // Toggle button text
  html += "<form action='/toggle' method='POST'><button class='" + buttonClass + "'>" + buttonText + "</button></form>";

  html += "</body></html>";
  server.send(200, "text/html", html);
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
    relayState = HIGH;
  } else if (message == "OFF") {
    digitalWrite(relayPin, LOW);
    relayState = LOW;
  } else if (message == "IP") {
    
    sendIPAddress();
    Serial.println(WiFi.localIP());
  }
}

void sendIPAddress() {
  if (WiFi.status() == WL_CONNECTED) {
    IPAddress ip = WiFi.localIP();
    char ipStr[32]; // Buffer to hold the formatted IP string
    snprintf(ipStr, sizeof(ipStr), "IP:%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]); // Format as IP:192.168.0.123
    client.publish(mqtt_topic3, ipStr); // Publish the formatted IP to the topic "device/ip"
    Serial.print("Published IP Address: ");
    Serial.println(ipStr);
  }
}