#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <string.h>
#include <ESP32Ping.h>

WiFiClient espClient;
PubSubClient client(espClient);
 
String topic = "testing/miquel"; // Topic de la placa
const char* ssid = "MiFibra-D6FA"; // SSID Wifi         
const char* password = "fsEtk2oX"; // Contraseña Wifi
const char* mqtt_server = "84.120.156.93"; // IP del Broker
int mqtt_port = 1986; // Puerto del Broker

String mac;
int contador;
bool temp, hum, pres, light, ipsensor, configured;

//const char* ipArray[50];
String ipArray[30];

unsigned long lastMillis;
 
void setup() {
 Serial.begin(115200);
 setup_wifi();
 configureBoard();
 getDeviceIPs();
 client.setServer(mqtt_server, mqtt_port);
 client.setCallback(callback);
 
 Serial.print("Destination Broker: ");
 Serial.print(mqtt_server);
 Serial.print(":");
 Serial.println(String(mqtt_port));
 Serial.print("Board IP: ");
 Serial.println(WiFi.localIP());
 
}
 
void setup_wifi() {
 
 delay(10);
 // We start by connecting to a WiFi network
 Serial.println();
 Serial.print("Connecting to ");
 Serial.println(ssid);
 
 WiFi.begin(ssid, password);
 
 while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
 }
 
 Serial.println("");
 Serial.print("Connected to WiFi, IP address: ");
 Serial.print(WiFi.localIP());
 Serial.print(" | MAC Address: ");
 Serial.println(WiFi.macAddress());
}
 
void callback(char* topic, byte* payload, unsigned int length) {
 Serial.print("Message arrived [");
 Serial.print(topic);
 Serial.print("] ");
 for (int i = 0; i < length; i++) {
 Serial.print((char)payload[i]);
 }
}


String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void configureBoard() {
  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
    HTTPClient http;
    //http.begin("https://pastebin.com/raw/3t6usgge"); //Specify the URL
   // http.begin("http://90.77.235.236:8080/JerseyDemos/rest/Tablaplaca/24:62:AB:CA:A4:60");
    http.begin("http://90.77.235.236:8080/JerseyDemos/rest/Tablaplaca/" + WiFi.macAddress());
    int httpCode = http.GET();                                        //Make the request
    if (httpCode > 0) { //Check for the returning code
      if (httpCode != 200) {
        Serial.print("");
        Serial.println("Configuration not fullfilled, this board might not exist in the database");
        configured = false;   
        //
      } else {
        String payload = http.getString();
        topic = getValue(payload, ' ', 0);
        if (getValue(payload, ' ', 1).equalsIgnoreCase("TRUE")) {
        temp = true;
        } else {
          temp = false;
        } if (getValue(payload, ' ', 2).equalsIgnoreCase("TRUE")) {
          hum = true;
        } else {
          hum = false;
        } if (getValue(payload, ' ', 3).equalsIgnoreCase("TRUE")) {
          pres = true;
        } else {
          pres = false;
        } if (getValue(payload, ' ', 4).equalsIgnoreCase("TRUE")) {
          light = true;
        } else {
          light = false;
        } if (getValue(payload, ' ', 5).equalsIgnoreCase("TRUE")) {
          ipsensor = true;
        } else {
          ipsensor = false;
        }
        Serial.print("Configuration successful!");
        Serial.println("");
        Serial.print("Selected topic: ");
        Serial.println(topic);
        Serial.print("Is Temperature: ");
        Serial.println(temp);
        Serial.print("Is Humidity: ");
        Serial.println(hum);
        Serial.print("Is Presence: ");
        Serial.println(pres);
        Serial.print("Is Light: ");
        Serial.println(light);
        Serial.print("Is IP Sensor: ");
        Serial.println(ipsensor);
        Serial.print("");
        configured = true;
      }
      
  }
 
  else {
    Serial.println("Error on HTTP request");
    }
 
    http.end(); //Free the resources
  }
}

void getDeviceIPs() {
  bool pingTest;
  contador = 0;
  
  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
    HTTPClient http;
    http.begin("http://90.77.235.236:8080/JerseyDemos/rest/Tablaip/" + WiFi.macAddress());
    int httpCode = http.GET();                                        //Make the request
    if (httpCode > 0) { //Check for the returning code
      if (httpCode != 200) {
        Serial.print("");
        Serial.println("IPs that belong to this board's MAC address not found, are you sure it exists in the database?");
        //
      } else {
        String payload = http.getString();
        int arrayLength = getValue(payload, ' ', 0).toInt();

        for (int i = 0; i <= arrayLength; i++) {
          ipArray[i] = getValue(payload, ' ', i+1);
        }
        if (ipArray[0] == 0) {
          Serial.println("No ips have been inserted, please check that the database is correctly configured");
        } else {
          Serial.print("");
          Serial.println("IPs inserted");
        }
      }
    }
  }
}
 
void reconnect() {
 // Loop until we're reconnected
 while (!espClient.connected()) {
 Serial.print("Attempting connection to Broker...");
 // Attempt to connect
 if (client.connect("miquelalbur","test","test")) {
  Serial.println("connected");
  client.subscribe(topic.c_str());
 } else {
  Serial.println("Failed to connect, rc= ");
  Serial.print(client.state());
  Serial.print(" retrying in 5 seconds...");
  Serial.println("");
  // Wait 5 seconds before retrying
  delay(5000);
  }
 }
}
 
float getTemperature(){  //Get temperature
  float value = random(0,35);
  return value;
}
float getHumidity(){ //Get humidity
  float value = random(0,100);
  return value;
}
int getPresence(){ //Get presence
  int value = random (0,100);
  return value;
}

int getLight() {
  int value = random (0, 100);
  return value;
}
 
void loop() {
 String destinationString;
 
 //Publish Temperature
 if (temp && configured) {
   destinationString = topic + "/temperature";
   float tempS = getTemperature();
   char tempString[8];
   dtostrf(tempS, 1, 2, tempString);
   client.publish(destinationString.c_str(), tempString);
 }
 
 //Publish humidity
 if (hum && configured) {
   destinationString = topic + "/humidity";
   float humS = getHumidity();
   char humString[8];
   dtostrf(humS, 1, 2, humString);
   client.publish(destinationString.c_str(), humString);
 }
 
 
 //Publish presence
 if (pres && configured) {
   destinationString = topic + "/presence";
   int presS = getPresence();
   char presString[8];
   dtostrf(presS, 1, 2, presString);
   client.publish(destinationString.c_str(), presString);
 }

 // Publish light
 if (light && configured) {
    destinationString = topic + "/light";
    int lightS = getLight();
    char lightString[8];
    dtostrf(lightS, 1, 2, lightString);
    client.publish(destinationString.c_str(), lightString);
 }

// Publish connected devices
 if (ipsensor && configured && (millis() - lastMillis >= 2*60*1000UL)) {
  lastMillis = millis();
  contador = 0;
  destinationString = topic + "/devices";
  for(int i = 0; i <= 10; i++) {
    //Serial.print("IP:");
    //Serial.println(ipArray[i]);
    if (ipArray[i] != 0) {
      bool ipPing = Ping.ping(ipArray[i].c_str(),1);
      if (ipPing) {
        contador++;
      }
    }
  }
  String conDevices = String(contador);
  client.publish(destinationString.c_str(), conDevices.c_str());
 }

 if (!configured) {
  destinationString = "mollcontrol/board/setup";
  Serial.print("Board not configured, sending mac address to setup application...");
  Serial.println("");
  client.publish(destinationString.c_str(), WiFi.macAddress().c_str());
  Serial.print("Attempting re-configuration in a few seconds...");
  Serial.println("");
  delay(10000);
  configureBoard();
 }
 
 
 //destinationString = topic +  "/123";
 //client.publish(destinationString.c_str(), "Testing - 123");
 
 
 if (!espClient.connected()) {
  reconnect();
 }
 client.loop();
 delay(5000);
 
}
