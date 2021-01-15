# generator_arduino_controle
Thi code is a web based arduino controler for stoping and starting an Petrol Generator via a mobile app or Telegram.
This system only needs a reliable wifi witch can run from Generator battery or GSM modem that can run from Generator battery.
# Project Status
The web based generator controle is working and compleet but I add functionality as required.
## The Version 2 of the system
The new V2 version has remote telegram controle and mobile app controle interface.<br>
I also have changed the Android Framework to Tasmota.<br>
This system is using MQTT and Node-Red to controle the start and stop of the Generator via Telegram and Mobile app.
The Generator Controler allso sends alerts if the generator output power is down or up via Telegram and can then also be started via Telegram app or mobile app.
![esp2866 board](iot_wifi_relay_module.png?raw=true "ESP8266 board")<br>
Giga Technology (http://www.giga.co.za) is selling a ESP3266 RELAY and opto input board for controlling the Generator.
Giga Technology (http://www.giga.co.za) also has a n servo unit to controle the chocke of an Petrol generator engin for cold start.
here is the link to the document explaning the APmode for seting up wifi details at startup
https://randomnerdtutorials.com/wifimanager-with-esp8266-autoconnect-custom-parameter-and-manage-your-ssid-and-password/
# Diagram
![esp2866 diagram](wifi_relay_sch.jpeg?raw=true "ESP8266 board")<br>
## Connection Diagram
![Connection diagram](Generator_wireing_diagram_v1.jpg ?raw=true "Connection Diagram")<br>
# adding board in arduino
http://arduino.esp8266.com/stable/package_esp8266com_index.json
# Add the following libraeries in arduino
In the Arduino IDE, navigate to Sketch > Include Library > Add .ZIP Library.<br>
ArduinoJson-5.13.5.zip<br>
WiFiManager.zip<br>
install the following mqtt libraery from the libraery manager  <br>
https://github.com/knolleary/pubsubclient<br>
# Node-Red flow
![Noe RED diagram](generator_node_red_flow_1.png?raw=true "Node RED Diagram")<br>
![Noe RED diagram](generator_node_red_flow_2.png?raw=true "Node RED Diagram")<br>

ref: https://ucexperiment.wordpress.com/2016/12/18/yunshan-esp8266-250v-15a-acdc-network-wifi-relay-module/

