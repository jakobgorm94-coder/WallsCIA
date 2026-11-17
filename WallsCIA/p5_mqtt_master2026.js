const topic = "WallsCIA";
const afsenderID = "BlueWallWeb";
const espID = "BlueWallESP"; // skal matche senderID i esp32_bluewall.ino

// afsender, val1, val2 og val3 bliver sat i receiver.js, hver gang der kommer en besked


function setup() {
    createCanvas(200, 200);
    background(220);

}


function draw() {

    if (afsender == espID)  {
        if (val2 == 1)  {
        background(0, 255, 0);
        //knap 23 på ESP'en er trykket og jeg afspiller en historie
        }
        if (val3 == 1)  {
        background(255, 0, 0);
        //knap 22 på ESP'en er trykket - begynder at optage historie
        }
        // ESP'en sender kun val1-val3 (to knapper), så der findes ingen val4 endnu.
        // Stop af optagelse kræver en tredje knap/værdi i ESP-koden.
    }
    // Hver besked fra ESP'en overskriver val2 og val3, så den anden værdi
    // bliver automatisk sat tilbage til 0, når der trykkes på en ny knap.

    }
