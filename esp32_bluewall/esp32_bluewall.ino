// ----MOJA 2024, 2026---------
#include <WiFi.h>
#include <MQTT.h> //v. 2.5.3
#include <ArduinoJson.h> //v. 7.4.3

// -- some needed global variables ---
WiFiClient net;
MQTTClient client(2048);
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

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, pass);
  connectWifi();
  connectMQTT();
}


void loop() 
{
  // DO YOUR THING HERE


  sender("hej", 5, 6);
  

  delay(1000);

  sender("davs", 55, 66);

  delay(1000);

  sender("ok", 1, 2);

  delay(1000);



  
  client.loop(); //keep MQTT running
}
