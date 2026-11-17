const topic = "WallsCIA";
const afsenderID = "BlueWallWeb";
//const espID = "BlueWallESP"; // skal matche senderID i esp32_bluewall.ino

// afsender, val1, val2, val3 og val4 bliver sat i receiver.js, hver gang der kommer en besked

// ---- TALE TIL TEKST (fra voice_listen) ----
let speechRec = new p5.SpeechRec('da-DK', gotSpeech);
let transcript = "";      // al tekst, der er optaget indtil nu
let optager = false;      // true mens vi optager
let sidsteHistorie = "";  // den seneste historie, der blev sendt

// ---- TEKST TIL TALE (fra talerp5) ----
let mySpeech = new p5.Speech(); // Speech synthesis object
mySpeech.interrupt = true;      // et nyt tryk starter forfra i stedet for at stå i kø


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
        if (val4 == 1)  {
            //knap 21 på ESP'en er trykket - læser den optagede historie højt
            afspilHistorie();
        }
    }
}


function draw() {

    if (optager) {
        background(255, 0, 0);
        text("Optager...\n" + transcript, 20, 20, width - 40, height - 40);
    } else if (window.speechSynthesis.speaking) {
        background(0, 255, 0);
        text("Afspiller...\n" + sidsteHistorie, 20, 20, width - 40, height - 40);
    } else {
        background(220);
        text(sidsteHistorie, 20, 20, width - 40, height - 40);
    }

    // Browseren må først afspille lyd, når nogen har klikket på siden
    if (navigator.userActivation && !navigator.userActivation.hasBeenActive) {
        text("Klik på siden for at aktivere lyd", 20, height - 40);
    }
}


// Læser den seneste optagede historie højt med en dansk stemme
function afspilHistorie() {
    if (optager) {
        console.log("Kan ikke afspille, mens der optages");
        return;
    }
    if (sidsteHistorie == "") {
        console.log("Der er ingen historie at afspille endnu");
        return;
    }
    let stemme = findDanskStemme();
    if (stemme) {
        mySpeech.utterance.voice = stemme;
        console.log("Afspiller med stemmen: " + stemme.name);
    } else {
        console.log("Ingen dansk stemme fundet - bruger browserens standardstemme");
    }
    mySpeech.utterance.lang = "da-DK";
    mySpeech.speak(sidsteHistorie);
}

// Finder en dansk stemme i browseren (helst en af de naturlige stemmer i Edge).
// Stemmerne er forskellige fra computer til computer, så vi vælger efter sprog
// og ikke efter et fast nummer.
function findDanskStemme() {
    let danske = window.speechSynthesis.getVoices().filter(s => s.lang.toLowerCase().startsWith("da"));
    let naturlig = danske.find(s => s.name.includes("Natural"));
    return naturlig || danske[0];
}


function startOptagelse() {
    if (optager) {
        return; // vi optager allerede
    }
    mySpeech.cancel();            // stop evt. oplæsning, så den ikke bliver optaget
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
        sender(sidsteHistorie, 0, 0, 0); // historien sendes som val1
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
