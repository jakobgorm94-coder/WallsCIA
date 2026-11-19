void modtager(String &topic, String &indhold) 
{
  deserializeJson(recieverBuffer,indhold);
  String afsender = recieverBuffer["from"];
  String value1 = recieverBuffer["val1"];
  int value2 = recieverBuffer["val2"];
  int value3 = recieverBuffer["val3"];
  int value4 = recieverBuffer["val4"];
  Serial.println(value1);
  Serial.println(value2);
  Serial.println(value3);
  Serial.println(value4);
  //do something with the value... eg change a global value
 // x = value;
 
}
