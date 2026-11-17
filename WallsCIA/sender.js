// -- HANDLE WHEN YOU SEND A MESSAGE----------------


function sender(x, y, z) {
  besked = {
    from:afsenderID, 
    val1:x,
    val2:y,
    val3:z
  };
   client.publish(topic, JSON.stringify(besked));
}
