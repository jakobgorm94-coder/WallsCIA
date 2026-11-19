void sender(String x, int y, int z, int w)
{
  senderBuffer["from"]=senderID;
  senderBuffer["val1"]=x;
  senderBuffer["val2"]=y;
  senderBuffer["val3"]=z;
  senderBuffer["val4"]=w;
  String JSONbesked;
  serializeJson(senderBuffer, JSONbesked);
  client.publish(topic, JSONbesked);
  delay(100); 
}
