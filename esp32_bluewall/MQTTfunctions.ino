//####################### MQTT FUNCTIONS #######################

void connectWifi() //--------------------------------------------
{
   while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");
    delay(1000);
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
    delay(1000);
  }
  Serial.println("MQTT OK");

  client.subscribe(topic);
}
