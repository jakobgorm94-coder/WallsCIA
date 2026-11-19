// -- HANDLE WHEN A MESSAGE ARRIVES----------------

// Seneste besked. Globale, så sketchen kan læse dem.
let afsender = "";
let val1 = "";
let val2 = 0;
let val3 = 0;
let val4 = 0;

client.on("message", modtager);

function modtager(modtagetTopic, modtagetBesked) {
    let modtagerBuffer;
    try {
        modtagerBuffer = JSON.parse(modtagetBesked);
    } catch (e) {
        // Brokeren er offentlig, så der kan komme beskeder der ikke er JSON
        console.log("Ignorerer besked der ikke er JSON:", modtagetBesked.toString());
        return;
    }
    console.log(modtagerBuffer);

    // Vi modtager også vores egne beskeder - dem springer vi over.
    // Derfor gemmer en væg aldrig sin egen historie.
    if (modtagerBuffer.from == afsenderID) {
        return;
    }

    afsender = modtagerBuffer.from;

    if (modtagetTopic == historieTopic) {
        // En historie fra en anden væg
        historieModtaget(modtagerBuffer.historie); // ligger i p5_mqtt_master2026.js
        return;
    }

    // Et knaptryk fra en ESP
    val1 = modtagerBuffer.val1;
    val2 = modtagerBuffer.val2;
    val3 = modtagerBuffer.val3;
    val4 = modtagerBuffer.val4;
    beskedModtaget(); // ligger i p5_mqtt_master2026.js
}
