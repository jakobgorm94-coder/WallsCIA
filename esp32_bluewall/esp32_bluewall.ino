// ----MOJA 2024, 2026---------
#include <WiFi.h>
#include <MQTT.h> //v. 2.5.3
#include <ArduinoJson.h> //v. 7.4.3

// -- some needed global variables ---
WiFiClient net;
MQTTClient client;
JsonDocument senderBuffer;
JsonDocument recieverBuffer;
// ------------------------------------

// ########### WIFI CONFIG ###########
const char ssid[] = "sensors";
const char pass[] = "FGeRiGzg";

// ########### MQTT CONFIG ###########
const String topic = "WallsCIA";
const String senderID = "BlueWallESP"; //giver mulighed for at skelne mellem afsendere

// ________NOW FOLLOWS MAIN PROGRAMS______

int buttonPin23 = 23;
int buttonPin22 = 22;
int buttonPin21 = 21; // ny knap - ret tallet, hvis den sidder på en anden pin

bool lastbuttonstate23 = HIGH;
bool lastbuttonstate22 = HIGH;
bool lastbuttonstate21 = HIGH;



void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, pass);
  connectWifi();
  connectMQTT();

  pinMode(buttonPin23, INPUT_PULLUP);
  pinMode(buttonPin22, INPUT_PULLUP);
  pinMode(buttonPin21, INPUT_PULLUP);

  
}


void loop() 
{
  bool currentbuttonstate23 = digitalRead(buttonPin23);
  bool currentbuttonstate22 = digitalRead(buttonPin22);
  bool currentbuttonstate21 = digitalRead(buttonPin21);

  if (lastbuttonstate23 == HIGH && currentbuttonstate23 == LOW) {
    Serial.println("knap 23 trykket");

    sender("hej", 1, 0, 0);
  }
  lastbuttonstate23 = currentbuttonstate23;
  if (lastbuttonstate22 == HIGH && currentbuttonstate22 == LOW) {
    Serial.println("knap 22 trykket");

    sender("hej", 0, 1, 0);
  }
  lastbuttonstate22 = currentbuttonstate22;
  if (lastbuttonstate21 == HIGH && currentbuttonstate21 == LOW) {
    Serial.println("knap 21 trykket");

    sender("hej", 0, 0, 1);
  }
  lastbuttonstate21 = currentbuttonstate21;



  
  client.loop(); //keep MQTT running
}
