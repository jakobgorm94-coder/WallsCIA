// -- HANDLE WHEN A MESSAGE ARRIVES----------------

client.on("message", modtager);

function modtager(topic, modtagetBesked) {
    let modtagerBuffer = JSON.parse(modtagetBesked);
    console.log(modtagerBuffer);
    
    let afsender = modtagerBuffer.from;
    let value1 = modtagerBuffer.val1;
    let value2 = modtagerBuffer.val2;
    let value3 = modtagerBuffer.val3;
}
