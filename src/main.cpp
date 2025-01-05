#include <Arduino.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "Secrets.h"

#define M0 5
#define M1 6
#define AUX 4

#define TXD2 19 
#define RXD2 18

#define ARRIVED 0x55
#define EMPTY 0xAA
#define ACKNOWLEDGE 0x25

byte receivedCode = 0;
bool transmissionSuccess = 0;
String macAddr;
String uniqueID;
String messageTopic;
String discoveryTopic;

enum boxStatus {
  empty,
  full
};

boxStatus mailBoxStatus = empty;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if a '1' was received as the first character
  if ((char)payload[0] == '1') {
    digitalWrite(2, HIGH);  // Turn the LED off (active low)
  } else {
    digitalWrite(2, LOW);  // Turn the LED on
  }
}

void reconnect() {

  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "Mailbox-";
    clientId += String(random(0xffff), HEX);
    Serial.println(clientId);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pw)) {
      Serial.println("connected");
      //client.publish(messageTopic.c_str(), "reconnected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 10 seconds");
      delay(10000);
    }
  }
}

// Function to send MQTT discovery message
void mqtt_discovery() {
  
  if (!client.connected()) {
    reconnect();
  }

  JsonDocument doc;  // Allocate memory for the JSON document

  doc["name"] = "LoraHub";
  doc["device_class"] = "security";
  doc["state_topic"] = "homeassistant/binary_sensor/" + uniqueID + "/state";
  doc["unique_id"] = uniqueID;  // Use MAC-based unique ID

  JsonObject device = doc["device"].to<JsonObject>();
  device["ids"] = macAddr;      // Use the full MAC address as identifier
  device["name"] = "Mailbox";   // Device name
  device["mf"] = "hewmorist";  // Include supplier info
  device["mdl"] = "Hub";
  device["sw"] = "1.0";
  device["hw"] = "0.1";
  
  char buffer[256];
  serializeJson(doc, buffer);  // Serialize JSON object to buffer

  Serial.println(discoveryTopic.c_str());
  Serial.println(buffer);                                // Print the JSON payload to Serial Monitor
  client.publish(discoveryTopic.c_str(), buffer, true);  // Publish to MQTT with retain flag set
  //client.publish(discoveryTopic.c_str(), buffer, false);  // Publish to MQTT with retain flag clear
}

void setup() {

  HardwareSerial Serial2(1);

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  pinMode(AUX, INPUT_PULLUP); 
  delay(1000); 

  Serial.begin(115200);
  delay(1000);
  Serial.println("Begin Setup");

  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setBufferSize(512);
  client.setCallback(callback);
  

  digitalWrite(M0, HIGH);
  digitalWrite(M1, HIGH);
  delay(100);

  byte data[] = { 0xC0, 0x0, 0x1, 0x1D, 0x34, 0x40 }; //914 MHz, Chan 1, 9.6k uart, 19.2k air
  for (int i = 0; i < sizeof(data); i++) {
    Serial2.write(data[i]);
    Serial.println(data[i], HEX);
  }
  delay(10);
  Serial.println("Starting");
  //while (digitalRead(AUX) == LOW)
   // ;

    Serial.println("AUX went High");
  digitalWrite(M0, LOW);
  digitalWrite(M1, LOW);
  delay(1000);
  Serial2.flush();

  // Get the MAC address of the board
  macAddr = WiFi.macAddress();
  Serial.println(macAddr);
  String hi = macAddr;
  hi.toLowerCase();
  hi.replace(":", "");  // Remove colons from MAC address to make it topic-friendly
  // Extract the last 6 characters of the MAC address (ignoring colons)
  uniqueID = "mailbox" + hi.substring(hi.length() - 4);  // Use last 4 byte

  messageTopic = "homeassistant/binary_sensor/" + uniqueID + "/state";
  discoveryTopic = "homeassistant/binary_sensor/" + uniqueID + "/config";
  Serial.println("Sending discovery MQTT");
  // Send MQTT discovery message
  mqtt_discovery(); 

  Serial.println("Init finished");
}

void loop() {

   Serial.println("Begin Loop");
  
  /* if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (Serial2.available() > 0) {
    while (Serial2.available() > 0) {

      receivedCode = Serial2.read();
      Serial.print(receivedCode, HEX);

      if (receivedCode == ARRIVED) {
        transmissionSuccess = true;
        mailBoxStatus = full;
        Serial.println("Mailbox Full");
        // Publish the new mailbox state to the desired topic
        client.publish(messageTopic.c_str(), "ON", true);  // Update mailbox status with "ON"
      }

      if (receivedCode == EMPTY) {
        transmissionSuccess = true;
        mailBoxStatus = empty;
        Serial.println("Mailbox empty");
        client.publish(messageTopic.c_str(), "OFF", true);  // Update mailbox status
      }
    }
  }

  if (transmissionSuccess) {
    Serial2.write(ACKNOWLEDGE);
    Serial.println(mailBoxStatus);
    transmissionSuccess = false;
    Serial.println("Transmission acknowledged");
  } */
 
 delay(1000);
 
}