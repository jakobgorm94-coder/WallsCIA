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

// Sender en optaget historie til de andre vægge.
// Historier har deres eget topic, så ESP'erne ikke modtager de lange tekster.
function sendHistorie(tekst) {
  let besked = {
    from:afsenderID,
    historie:tekst
  };
  client.publish(historieTopic, JSON.stringify(besked));
}
