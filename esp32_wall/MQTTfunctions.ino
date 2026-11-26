//####################### MQTT FUNCTIONS #######################
// Forbindelsen til WiFi og MQTT bliver holdt ved lige fra loop() uden at stoppe
// programmet. Så bliver knapper og sensor ved med at virke, mens ESP'en forbinder.

bool wifiVarForbundet = false;       // var WiFi forbundet, sidst vi kiggede?
bool mqttVarForbundet = false;       // var MQTT forbundet, sidst vi kiggede?
unsigned long sidsteWifiStart = 0;   // hvornår vi sidst startede et forsøg på WiFi
unsigned long sidstePrik = 0;        // hvornår vi sidst skrev en prik
unsigned long sidsteMqttForsoeg = 0; // hvornår vi sidst prøvede at forbinde til MQTT

// Skriver årsagen, hver gang et forsøg på at forbinde til WiFi mislykkes.
// På sensors-netværket ser vi ofte 2 (netværket svarede ikke i tide) et antal
// gange, før det lykkes - det er normalt, ESP'en prøver selv igen.
void wifiHaendelse(WiFiEvent_t event, WiFiEventInfo_t info)
{
  if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
    Serial.println();
    Serial.print("WiFi afbrudt, aarsag: ");
    Serial.println(info.wifi_sta_disconnected.reason);
  }
}

// Kaldes én gang fra setup()
void startForbindelse() //--------------------------------------------
{
  client.begin("broker.hivemq.com", net);
  client.onMessage(modtager);

  WiFi.onEvent(wifiHaendelse);
  WiFi.begin(ssid, pass);
  sidsteWifiStart = millis();
}

// Prøver at forbinde til MQTT. Tager normalt under et sekund.
void connectMQTT() //--------------------------------------------
{
  //-- Generate a Unique User ID 
  String dummy = "ITU";
  const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  for (int i = 0; i < 9; i++) {
    dummy += chars[random(0, sizeof(chars) - 1)];
  }
  int str_len = dummy.length()+1;
  char userID[str_len];
  dummy.toCharArray(userID,str_len);

  //--- DO THE MQTT CONNECT -----
  if (client.connect(userID)) {
    mqttVarForbundet = true;
    Serial.println("MQTT OK");
    client.subscribe(topic);
  }
}

// Kaldes hele tiden fra loop()
void vedligeholdForbindelse() //--------------------------------------------
{
  // ---- WiFi ----
  if (WiFi.status() != WL_CONNECTED) {
    if (wifiVarForbundet) {
      wifiVarForbundet = false;
      mqttVarForbundet = false;
      Serial.println("WiFi-forbindelsen er tabt - forbinder igen");
      sidsteWifiStart = millis(); // ESP'en prøver selv at forbinde igen
    }
    if (millis() - sidstePrik >= 1000) {
      sidstePrik = millis();
      Serial.print(".");
    }
    if (millis() - sidsteWifiStart >= 15000) {
      // 15 sekunder uden forbindelse: start forbindelsen helt forfra
      Serial.println();
      Serial.println("Stadig ingen WiFi - proever forfra");
      WiFi.disconnect();
      delay(100);
      WiFi.begin(ssid, pass);
      sidsteWifiStart = millis();
    }
    return; // uden WiFi kan vi ikke forbinde til MQTT
  }
  if (!wifiVarForbundet) {
    wifiVarForbundet = true;
    Serial.println();
    Serial.println("WIFI OK");
    sidsteMqttForsoeg = 0; // prøv MQTT med det samme
  }

  // ---- MQTT ----
  if (!client.connected()) {
    if (mqttVarForbundet) {
      mqttVarForbundet = false;
      Serial.println("MQTT-forbindelsen er tabt - forbinder igen");
    }
    // Prøv højst hvert 2. sekund, så knapperne ikke bliver blokeret af forsøgene
    if (sidsteMqttForsoeg == 0 || millis() - sidsteMqttForsoeg >= 2000) {
      sidsteMqttForsoeg = max(millis(), 1UL);
      connectMQTT();
    }
  }
}
