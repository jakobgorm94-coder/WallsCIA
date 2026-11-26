//####################### AFSTANDSSENSOR OG LED-STRIP #######################
// Afstandssensoren (HC-SR04) måler, om nogen står tæt på væggen. Når der gør,
// lyser LED-strippen i én farve, og websiden får besked, så den kan
// sige "psst". Indstillingerne (pins, afstand, antal LED'er) står øverst i
// esp32_wall.ino.

bool nogenNaer = false;            // true når nogen står tæt på væggen
int naerTaeller = 0;               // antal målinger i træk med nogen tæt på
int vaekTaeller = 0;               // antal målinger i træk uden nogen
unsigned long sidsteMaaling = 0;   // tidspunkt for seneste afstandsmåling
unsigned long sidsteNaerBesked = 0;// tidspunkt for seneste "naer"-besked
unsigned long sidsteLysTrin = 0;   // tidspunkt for seneste opdatering af lyset
bool optagelseIGang = false;       // true fra optage-knappen er trykket, til stop-knappen trykkes
bool historieOptaget = false;      // true når personen foran væggen har optaget en historie
bool lysTaendt = false;            // om strippen lyser lige nu (bruges til udskrift)
unsigned long sidsteUdskrift = 0;  // tidspunkt for seneste udskrift af afstanden

void opsaetNaerhed()
{
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(trigPin, LOW);

  FastLED.addLeds<LED_TYPE, LED_PIN, FARVE_RAEKKEFOELGE>(leds, ANTAL_LEDS);
  FastLED.setBrightness(LYSSTYRKE);
  FastLED.clear();
  FastLED.show(); // sluk strippen fra start
}

// Måler afstanden i cm. Giver 0, hvis sensoren ikke får noget ekko tilbage
// (ingen inden for ca. 4 meter).
int maalAfstand()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  unsigned long varighed = pulseIn(echoPin, HIGH, 25000); // vent højst 25 ms på ekkoet
  return varighed / 58; // lydens rejsetid frem og tilbage omregnet til cm
}

// Fortæller websiden, om der står nogen ("naer") eller ej ("vaek").
// Sender direkte uden ventetiden i sender(), så lyset ikke hakker.
void sendNaerhed(bool naer)
{
  senderBuffer["from"]=senderID;
  senderBuffer["val1"]=naer ? "naer" : "vaek";
  senderBuffer["val2"]=0;
  senderBuffer["val3"]=0;
  senderBuffer["val4"]=0;
  String JSONbesked;
  serializeJson(senderBuffer, JSONbesked);
  client.publish(topic, JSONbesked);
  sidsteNaerBesked = millis();
}

// Tænder alle LED'er i væggens farve eller slukker dem
void opdaterLys()
{
  sidsteLysTrin = millis();
  bool skalLyse = nogenNaer && !historieOptaget;
  if (skalLyse != lysTaendt) {
    lysTaendt = skalLyse;
    Serial.println(lysTaendt ? "Lys: TAENDT" : "Lys: SLUKKET");
  }
  if (skalLyse) {
    fill_solid(leds, ANTAL_LEDS, LYS_FARVE);
  } else {
    fill_solid(leds, ANTAL_LEDS, CRGB::Black); // slukket
  }
  FastLED.show();
}

// Kaldes fra loop(), når optage-knappen (knap 2) trykkes
void optagelseStartet()
{
  optagelseIGang = true;
}

// Kaldes fra loop(), når stop-knappen (knap 3) trykkes. Hvis der var en optagelse
// i gang, er historien nu optaget, og lyset slukkes, indtil personen er gået.
void optagelseAfsluttet()
{
  if (optagelseIGang) {
    optagelseIGang = false;
    historieOptaget = true;
    Serial.println("Historie optaget - lyset slukkes");
    opdaterLys();
  }
}

// Venter et antal millisekunder, mens sensoren og lyset bliver ved med at køre.
// Bruges, mens ESP'en forbinder til WiFi, så væggen lyser, selvom nettet er langsomt.
void ventMedSensor(unsigned long ms)
{
  unsigned long start = millis();
  while (millis() - start < ms) {
    opdaterNaerhed();
    delay(5);
  }
}

// Kaldes hele tiden fra loop()
void opdaterNaerhed()
{
  // ---- Mål afstanden 10 gange i sekundet ----
  if (millis() - sidsteMaaling >= 100) {
    sidsteMaaling = millis();
    int afstand = maalAfstand();

    // Vis afstanden en gang i sekundet, så man kan se, at sensoren virker
    if (VIS_AFSTAND && millis() - sidsteUdskrift >= 1000) {
      sidsteUdskrift = millis();
      if (afstand == 0) {
        Serial.println("Afstand: intet ekko (ingen inden for 4 meter, eller sensoren er ikke forbundet)");
      } else {
        Serial.print("Afstand: ");
        Serial.print(afstand);
        Serial.println(" cm");
      }
    }

    if (afstand > 0 && afstand < NAER_CM) {
      naerTaeller++;
      vaekTaeller = 0;
    } else if (afstand == 0 || afstand > VAEK_CM) {
      vaekTaeller++;
      naerTaeller = 0;
    }

    // Sensoren kan måle forkert en enkelt gang. Derfor skal der 3 målinger i træk
    // til, før nogen "er kommet", og 1,5 sekund uden nogen, før de "er gået".
    if (!nogenNaer && naerTaeller >= 3) {
      nogenNaer = true;
      Serial.print("Nogen er taet paa: ");
      Serial.print(afstand);
      Serial.println(" cm");
      sendNaerhed(true);
      opdaterLys(); // tænd lyset med det samme
    }
    if (nogenNaer && vaekTaeller >= 15) {
      nogenNaer = false;
      Serial.println("Ingen i naerheden");
      sendNaerhed(false);
      // Næste person, der kommer, skal have lys igen
      historieOptaget = false;
      optagelseIGang = false;
      opdaterLys(); // sluk lyset med det samme
    }
  }

  // ---- Mens nogen står der: gentag beskeden hvert 3. sekund ----
  // Så ved websiden det også, hvis den først bliver åbnet bagefter.
  if (nogenNaer && millis() - sidsteNaerBesked >= 3000) {
    sendNaerhed(true);
  }

  // ---- Lyset: tændt mens nogen står der, indtil de har optaget en historie ----
  // Farven sendes igen to gange i sekundet, så strippen retter sig selv,
  // hvis en LED skulle have fået en forkert farve.
  if (millis() - sidsteLysTrin >= 500) {
    opdaterLys();
  }
}
