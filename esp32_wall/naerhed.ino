//####################### AFSTANDSSENSOR #######################
// Afstandssensoren (HC-SR04) måler, om nogen står tæt på væggen. Når der gør,
// får websiden besked, så den kan sige "psst". Indstillingerne (pins og afstand)
// står øverst i esp32_wall.ino.

bool nogenNaer = false;            // true når nogen står tæt på væggen
int naerTaeller = 0;               // antal målinger i træk med nogen tæt på
int vaekTaeller = 0;               // antal målinger i træk uden nogen
unsigned long sidsteMaaling = 0;   // tidspunkt for seneste afstandsmåling
unsigned long sidsteNaerBesked = 0;// tidspunkt for seneste "naer"-besked
unsigned long sidsteUdskrift = 0;  // tidspunkt for seneste udskrift af afstanden

void opsaetNaerhed()
{
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(trigPin, LOW);
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
// Sender direkte uden ventetiden i sender(), så målingerne ikke forsinkes.
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

// Venter et antal millisekunder, mens sensoren bliver ved med at køre.
// Bruges, mens ESP'en forbinder til WiFi, så sensoren virker, selvom nettet er langsomt.
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
    }
    if (nogenNaer && vaekTaeller >= 15) {
      nogenNaer = false;
      Serial.println("Ingen i naerheden");
      sendNaerhed(false);
    }
  }

  // ---- Mens nogen står der: gentag beskeden hvert 3. sekund ----
  // Så ved websiden det også, hvis den først bliver åbnet bagefter.
  if (nogenNaer && millis() - sidsteNaerBesked >= 3000) {
    sendNaerhed(true);
  }
}
