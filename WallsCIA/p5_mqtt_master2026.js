const topic = "WallsCIA";
const afsenderID = "BlueWallWeb";
//const espID = "BlueWallESP"; // skal matche senderID i esp32_bluewall.ino

// afsender, val1, val2 og val3 bliver sat i receiver.js, hver gang der kommer en besked

// ---- TALE TIL TEKST (fra voice_listen) ----
let speechRec = new p5.SpeechRec('da-DK', gotSpeech);
let transcript = "";      // al tekst, der er optaget indtil nu
let optager = false;      // true mens vi optager
let sidsteHistorie = "";  // den seneste historie, der blev sendt


function setup() {
    createCanvas(400, 400);
    textSize(24);
    textWrap(WORD);               // linjeskift mellem ord

    speechRec.onEnd = optagelseSlut;
    speechRec.onError = optagelseFejl;
}


// Bliver kaldt fra receiver.js, hver gang der kommer en ny besked.
// Knaptryk håndteres her (og ikke i draw), så de kun udføres én gang pr. tryk
// og også virker, når browserfanen ligger i baggrunden.
function beskedModtaget() {
    if (afsender == "BlueWallESP")  {
        console.log("BlueWallESP YES");
        if (val3 == 1)  {
            //knap 22 på ESP'en er trykket - begynder at optage historie
            startOptagelse();
        }
        if (val2 == 1)  {
            //knap 23 på ESP'en er trykket - stopper optagelse og sender historien
            stopOptagelse();
        }
    }
}


function draw() {

    if (optager) {
        background(255, 0, 0);
        text("Optager...\n" + transcript, 20, 20, width - 40, height - 40);
    } else {
        background(220);
        text(sidsteHistorie, 20, 20, width - 40, height - 40);
    }
}


function startOptagelse() {
    if (optager) {
        return; // vi optager allerede
    }
    transcript = "";
    optager = true;
    speechRec.start(true, false); // continuous = true, interim = false
    console.log("Optagelse startet");
}

function stopOptagelse() {
    if (!optager) {
        return; // der er ikke noget at stoppe
    }
    optager = false;
    speechRec.rec.stop();         // send IKKE her, vent på optagelseSlut
    console.log("Optagelse stoppet");
}

// Bliver kaldt, når talegenkendelsen stopper
function optagelseSlut() {
    if (optager) {
        // Browseren stopper selv efter en pause i talen - start igen, indtil stop-knappen trykkes
        speechRec.start(true, false);
        return;
    }
    if (transcript.trim() !== "") {
        sidsteHistorie = transcript.trim();
        sender(sidsteHistorie, 0, 0); // historien sendes som val1
        console.log("Historie sendt: " + sidsteHistorie);
    }
    transcript = "";
}

function optagelseFejl(fejl) {
    console.log("Fejl i talegenkendelse: " + fejl.error);
    if (fejl.error == "not-allowed" || fejl.error == "service-not-allowed" || fejl.error == "audio-capture") {
        // Ingen adgang til mikrofonen - stop, så vi ikke prøver igen og igen
        optager = false;
    }
}

function gotSpeech() {
    if (speechRec.resultValue) {
        transcript += speechRec.resultString + " "; // læg til i stedet for at overskrive
        console.log(transcript);                    // hele teksten kan også ses i konsollen
    }
}
