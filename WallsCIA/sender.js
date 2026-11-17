// -- HANDLE WHEN YOU SEND A MESSAGE----------------


function sender(x, y, z, w) {
  besked = {
    from:afsenderID, 
    val1:x,
    val2:y,
    val3:z,
    val4:w
  };
   client.publish(topic, JSON.stringify(besked));
}
