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

// ########### HVILKEN VÆG? ###########
// Koden er ens på alle tre ESP'er - kun denne linje skal rettes, før der uploades.
// Skriv "Red", "Blue" eller "Yellow" (præcis sådan, med stort begyndelsesbogstav).
const String vaeg = "Blue";
const String senderID = vaeg + "WallESP"; //fx "BlueWallESP" - websiden lytter kun til sin egen vægs ESP

// ________NOW FOLLOWS MAIN PROGRAMS______

int buttonPin21 = 21; // knap 1: afspil en historie
int buttonPin22 = 22; // knap 2: start optagelse
int buttonPin23 = 23; // knap 3: stop optagelse og send historien

// Et tryk tæller først, når knappen har været holdt stabilt nede i debounceTid
// millisekunder. Så bliver støj og "prel" på pinnen ikke til falske tryk.
const unsigned long debounceTid = 50;

// For hver knap: den godkendte (stabile) tilstand, den sidst læste tilstand,
// og hvornår den læste tilstand sidst ændrede sig
bool stabil23 = HIGH;
bool stabil22 = HIGH;
bool stabil21 = HIGH;
bool sidstLaest23 = HIGH;
bool sidstLaest22 = HIGH;
bool sidstLaest21 = HIGH;
unsigned long sidstAendret23 = 0;
unsigned long sidstAendret22 = 0;
unsigned long sidstAendret21 = 0;



void setup() {
  Serial.begin(9600);
  WiFi.onEvent(wifiHaendelse); // skriver årsagen, hvis WiFi ikke kan forbinde
  WiFi.begin(ssid, pass);
  connectWifi();
  connectMQTT();

  pinMode(buttonPin23, INPUT_PULLUP);
  pinMode(buttonPin22, INPUT_PULLUP);
  pinMode(buttonPin21, INPUT_PULLUP);

  
}

// Returnerer true præcis én gang pr. tryk: når knappen er gået fra sluppet til
// trykket og har været stabilt nede i debounceTid millisekunder.
bool knapTrykket(int pin, bool &stabil, bool &sidstLaest, unsigned long &sidstAendret)
{
  bool laest = digitalRead(pin);
  if (laest != sidstLaest) {
    sidstAendret = millis(); // tilstanden flakker - start ventetiden forfra
    sidstLaest = laest;
  }
  if (millis() - sidstAendret > debounceTid && laest != stabil) {
    stabil = laest;
    return stabil == LOW;
  }
  return false;
}


void loop()
{
  // Hvis forbindelsen er røget, forbinder vi igen - ellers virker knapperne ikke
  if (!client.connected()) {
    Serial.println("Forbindelsen er tabt - forbinder igen");
    connectWifi();
    connectMQTT();
  }

  if (knapTrykket(buttonPin23, stabil23, sidstLaest23, sidstAendret23)) {
    Serial.println("knap 23 trykket");

    sender("hej", 1, 0, 0);
  }
  if (knapTrykket(buttonPin22, stabil22, sidstLaest22, sidstAendret22)) {
    Serial.println("knap 22 trykket");

    sender("hej", 0, 1, 0);
  }
  if (knapTrykket(buttonPin21, stabil21, sidstLaest21, sidstAendret21)) {
    Serial.println("knap 21 trykket");

    sender("hej", 0, 0, 1);
  }



  
  client.loop(); //keep MQTT running
}
