// -- HANDLE WHEN A MESSAGE ARRIVES----------------

// Seneste besked fra ESP'en. Globale, så draw() i sketchen kan læse dem.
let afsender = "";
let val1 = "";
let val2 = 0;
let val3 = 0;

client.on("message", modtager);

function modtager(topic, modtagetBesked) {
    let modtagerBuffer;
    try {
        modtagerBuffer = JSON.parse(modtagetBesked);
    } catch (e) {
        // Brokeren er offentlig, så der kan komme beskeder der ikke er JSON
        console.log("Ignorerer besked der ikke er JSON:", modtagetBesked.toString());
        return;
    }
    console.log(modtagerBuffer);

    // Vi modtager også vores egne beskeder på samme topic - dem springer vi over
    if (modtagerBuffer.from == afsenderID) {
        return;
    }

    afsender = modtagerBuffer.from;
    val1 = modtagerBuffer.val1;
    val2 = modtagerBuffer.val2;
    val3 = modtagerBuffer.val3;
}
