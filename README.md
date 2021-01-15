# generator_arduino_controle
Thi code is a web based arduino controler for stoping and starting an Petrol Generator.
# Project Status
The web based generator controle is working.
I am now bussy adding mqtt controle to start and stop the Generator via Telegram and Mobile app.
I am using MQTT and Node Red to integrate to IFTTT with web hoeks and Telegram for events alering.
# Hardware.
![esp2866 board](iot_wifi_relay_module.png?raw=true "ESP8266 board")<br>
Giga Technology (http://www.giga.co.za) is selling a ESP3266 RELAY and opto input board for controlling the Generator.
Giga Technology (http://www.giga.co.za) also has a n servo unit to controle the chocke of an Petrol generator engin for cold start.
here is the link to the document explaning the APmode for seting up wifi details at startup
https://randomnerdtutorials.com/wifimanager-with-esp8266-autoconnect-custom-parameter-and-manage-your-ssid-and-password/
#Diagram
![esp2866 diagram](wifi_relay_sch.jpeg?raw=true "ESP8266 board")<br>
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

