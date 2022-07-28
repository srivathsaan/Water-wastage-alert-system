#include <Wire.h> 
#include "PubSubClient.h" // Connect and publish to the MQTT broker

// Code for the ESP8266
#include "ESP8266WiFi.h"  // Enables the ESP8266 to connect to the local network (via WiFi)
#define SENSOR  2
#define BUZ 5

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned long flowMilliLitres;
unsigned int totalMilliLitres;
float flowLitres;
float totalLitres;
void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}

// WiFi
const char* ssid = "waterman";                 // Your personal network SSID
const char* wifi_password = "66778899"; // Your personal network password

// MQTT
const char* mqtt_server = "192.168.250.56";  // IP of the MQTT broker
const char* flow_topic = "home/floor_1/flow_rate";
const char* output_topic = "home/floor_1/output";
const char* mqtt_username = "sv123"; // MQTT username
const char* mqtt_password = "sv123"; // MQTT password
const char* clientID = "client_home"; // MQTT client ID


// Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;
// 1883 is the listener port for the Broker
PubSubClient client(mqtt_server, 1883, wifiClient); 


// Custom function to connet to the MQTT broker via WiFi
void connect_MQTT(){
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // Connect to the WiFi
  WiFi.begin(ssid, wifi_password);

  // Wait until the connection has been confirmed before continuing
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Debugging - Output the IP Address of the ESP8266
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Connect to MQTT Broker
  // client.connect returns a boolean value to let us know if the connection was successful.
  // If the connection is failing, make sure you are using the correct MQTT Username and Password (Setup Earlier in the Instructable)
  if (client.connect(clientID, mqtt_username, mqtt_password)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }
}




void setup() {
  Serial.begin(9600); 
  pinMode(SENSOR, INPUT_PULLUP);
  pinMode(BUZ, OUTPUT);
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;
  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);
}
 
void loop() {
  currentMillis = millis();
  connect_MQTT();
  Serial.setTimeout(2000);
  
  if (currentMillis - previousMillis > interval)
  {
    pulse1Sec = pulseCount;
    pulseCount = 0;
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();
    flowMilliLitres = (flowRate / 60) * 1000;
    flowLitres = (flowRate / 60);
    totalMilliLitres += flowMilliLitres;
    totalLitres += flowLitres;
    
    Serial.print("Flow rate:");
    Serial.print(float(flowRate));
    Serial.print("L/min");
    Serial.print("\t"); 
    
    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(totalLitres);
    Serial.println("L");

 
  // MQTT can only transmit strings
  String hs="Flow Rate: "+String((float)flowRate)+" % ";
  String ts="Output Liquid: "+String((float)totalMilliLitres)+" C ";

  // PUBLISH to the MQTT Broker 
  if (client.publish(flow_topic, String(flowRate).c_str())) {
    Serial.println("Flow Rate sent!");
  }
  // Again, client.publish will return a boolean value depending on whether it succeded or not.
  // If the message failed to send, we will try again, as the connection may have broken.
  else {
    Serial.println("Flowrate failed to send. Reconnecting to MQTT Broker and trying again");
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
    client.publish(flow_topic, String(flowRate).c_str());
  }

  // PUBLISH to the MQTT Broker 
  if (client.publish(output_topic, String(totalMilliLitres).c_str())) {
    Serial.println("Total Millilitre sent!");
  }
  // Again, client.publish will return a boolean value depending on whether it succeded or not.
  // If the message failed to send, we will try again, as the connection may have broken.
  else {
    Serial.println("Total Millilitre failed to send. Reconnecting to MQTT Broker and trying again");
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
    client.publish(output_topic, String(totalMilliLitres).c_str());
  }
  client.disconnect();  // disconnect from the MQTT broker
  if(totalMilliLitres > 100.0){
    digitalWrite(BUZ, HIGH);
    delay(5000);
    digitalWrite(BUZ, LOW);
    totalMilliLitres = 0;
    delay(5000);
  }
  delay(1000*5);       // print new values every 20 seconds
}
}
