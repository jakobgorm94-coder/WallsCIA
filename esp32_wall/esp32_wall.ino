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

// ########### AFSTANDSSENSOR (HC-SR04) ###########
const int trigPin = 5;
const int echoPin = 18;
const int NAER_CM = 100; // nogen er "tæt på", når de er under 1 meter fra sensoren
const int VAEK_CM = 120; // de er først "gået" igen, når de er over 1,2 meter væk
const bool VIS_AFSTAND = true; // skriver afstanden i Serial Monitor hvert sekund. Sæt til false for at slå det fra

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



// Knaptryk, der venter på at blive sendt. Hvis forbindelsen er nede, når der
// trykkes, gemmes trykket her og sendes, så snart ESP'en er på nettet igen.
// Plads 0 = knap 23, plads 1 = knap 22, plads 2 = knap 21. 0 betyder "intet venter".
unsigned long ventendeTryk[3] = {0, 0, 0};
const unsigned long MAX_VENTETID = 8000; // tryk, der har ventet længere end 8 sekunder, droppes



void setup() {
  Serial.begin(9600);
  opsaetNaerhed(); // afstandssensor (ligger i naerhed.ino)

  pinMode(buttonPin23, INPUT_PULLUP);
  pinMode(buttonPin22, INPUT_PULLUP);
  pinMode(buttonPin21, INPUT_PULLUP);

  // Starter forbindelsen til WiFi og MQTT. Vi venter IKKE her på, at den er klar -
  // det klarer loop(), så knapper og sensor virker med det samme.
  startForbindelse(); // ligger i MQTTfunctions.ino
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

// Sender de knaptryk, der venter - men kun når vi er forbundet
void sendVentendeTryk()
{
  if (!client.connected()) {
    return; // trykkene bliver liggende, til forbindelsen er tilbage
  }
  for (int i = 0; i < 3; i++) {
    if (ventendeTryk[i] == 0) {
      continue; // intet tryk venter på denne knap
    }
    if (millis() - ventendeTryk[i] > MAX_VENTETID) {
      Serial.println("Et knaptryk blev droppet - forbindelsen var nede for laenge");
    } else if (i == 0) {
      sender("hej", 1, 0, 0); // knap 23: stop optagelse og send historien
    } else if (i == 1) {
      sender("hej", 0, 1, 0); // knap 22: start optagelse
    } else {
      sender("hej", 0, 0, 1); // knap 21: afspil en historie
    }
    ventendeTryk[i] = 0;
  }
}


void loop()
{
  // Holder WiFi og MQTT forbundet uden at stoppe resten af programmet
  vedligeholdForbindelse(); // ligger i MQTTfunctions.ino

  // Knapperne læses altid - også mens forbindelsen er nede
  if (knapTrykket(buttonPin23, stabil23, sidstLaest23, sidstAendret23)) {
    Serial.println("knap 23 trykket");
    ventendeTryk[0] = max(millis(), 1UL);
  }
  if (knapTrykket(buttonPin22, stabil22, sidstLaest22, sidstAendret22)) {
    Serial.println("knap 22 trykket");
    ventendeTryk[1] = max(millis(), 1UL);
  }
  if (knapTrykket(buttonPin21, stabil21, sidstLaest21, sidstAendret21)) {
    Serial.println("knap 21 trykket");
    ventendeTryk[2] = max(millis(), 1UL);
  }
  sendVentendeTryk();

  opdaterNaerhed(); // afstandssensor (ligger i naerhed.ino)

  client.loop(); //keep MQTT running
}
