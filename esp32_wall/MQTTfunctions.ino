//####################### MQTT FUNCTIONS #######################

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

void connectWifi() //--------------------------------------------
{
  int forsoeg = 0; // antal sekunder vi har ventet
   while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    ventMedSensor(1000); // vent 1 sekund - sensor og lys kører imens
    forsoeg++;
    if (forsoeg % 15 == 0 && WiFi.status() != WL_CONNECTED) {
      // 15 sekunder uden forbindelse: start forbindelsen helt forfra
      Serial.println();
      Serial.println("Stadig ingen WiFi - proever forfra");
      WiFi.disconnect();
      delay(500);
      WiFi.begin(ssid, pass);
    }
  }
  Serial.println
  ("WIFI OK");
}

void connectMQTT() //--------------------------------------------
{
  client.begin("broker.hivemq.com", net);
  client.onMessage(modtager);

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
  while (!client.connect(userID)){
    ventMedSensor(1000); // vent 1 sekund - sensor og lys kører imens
  }
  Serial.println("MQTT OK");

  client.subscribe(topic);
}
