const topic = "WallsCIA";                    // knaptryk fra ESP'erne
const historieTopic = "WallsCIA/historier";  // historier, der sendes mellem væggene

// ---- HVILKEN VÆG ER DETTE? ----
// Koden er ens på alle tre computere. Første gang siden åbnes, vælger man væg,
// og valget huskes i browseren. Man kan også vælge i adressen: index.html?wall=red
//
// Hver væg har sin egen danske stemme. "stemmer" er en ønskeliste: den første stemme,
// der findes på computeren, bliver brugt. "navn" er en del af stemmens navn, og
// "tonehoejde" gør stemmen dybere (under 1) eller lysere (over 1).
//
// Edge har tre danske stemmer (Christel, Jeppe og Helle). Chrome har kun Helle,
// så dér får væggene den samme stemme i tre forskellige tonehøjder.
const VAEGGE = {
    Red:    { navn: "Rød væg", farve: [220, 60, 60],
              stemmer: [{ navn: "Christel", tonehoejde: 1 }, { navn: "Helle", tonehoejde: 1.5 }] },
    Blue:   { navn: "Blå væg", farve: [70, 110, 230],
              stemmer: [{ navn: "Jeppe", tonehoejde: 1 }, { navn: "Helle", tonehoejde: 0.5 }] },
    Yellow: { navn: "Gul væg", farve: [240, 200, 40],
              stemmer: [{ navn: "Helle", tonehoejde: 1 }, { navn: "Jeppe", tonehoejde: 0.6 }] }
};
let vaeg = "";        // "Red", "Blue" eller "Yellow"
let afsenderID = "";  // denne sides navn i beskeder, fx "RedWallWeb"
let espID = "";       // navnet på denne vægs ESP, fx "RedWallESP" (senderID i esp32_wall.ino)
let vaegKnapper = []; // knapperne til at vælge væg med

// afsender, val1, val2, val3 og val4 bliver sat i receiver.js, hver gang der kommer en besked

// ---- HISTORIER ----
// Listen med de historier, denne væg har modtaget fra de andre vægge.
// Den gemmes i browseren, så den overlever genindlæsning og genstart.
let historier = [];        // hver historie: { tekst, fra, tid }
let sidstAfspillet = -1;   // nummeret på den historie, der sidst blev læst op
let aktuelHistorie = "";   // teksten, der læses op lige nu
let beskedTekst = "";      // kort besked, der vises på skærmen
let beskedSlut = 0;        // tidspunkt (millis), hvor beskeden forsvinder

// ---- TALE TIL TEKST (fra voice_listen) ----
let speechRec = new p5.SpeechRec('da-DK', gotSpeech);
let transcript = "";      // al tekst, der er optaget indtil nu
let optager = false;      // true mens vi optager

// ---- TEKST TIL TALE (fra talerp5) ----
let mySpeech = new p5.Speech(); // Speech synthesis object
let sidsteAfspilning = -1000;   // tidspunkt (millis) for seneste start af oplæsning


function setup() {
    createCanvas(400, 400);
    textSize(24);
    textWrap(WORD);               // linjeskift mellem ord

    speechRec.onEnd = optagelseSlut;
    speechRec.onError = optagelseFejl;

    findVaeg();
}


// ---------------- VALG AF VÆG ----------------

// Finder ud af hvilken væg denne computer er: først adressen (?wall=red),
// så det gemte valg, og ellers vises tre knapper at vælge med.
function findVaeg() {
    let fraAdresse = new URLSearchParams(window.location.search).get("wall");
    let gemt = hentGemt("WallsCIA-vaeg");
    let valg = vaegNavn(fraAdresse) || vaegNavn(gemt);
    if (valg) {
        vaelgVaeg(valg);
    } else {
        for (let v in VAEGGE) {
            let knap = createButton(VAEGGE[v].navn);
            knap.mousePressed(() => vaelgVaeg(v));
            vaegKnapper.push(knap);
        }
    }
}

// Oversætter fx "red" eller "RED" til "Red". Giver "" hvis det ikke er en væg.
function vaegNavn(tekst) {
    for (let v in VAEGGE) {
        if (tekst && tekst.toLowerCase() == v.toLowerCase()) {
            return v;
        }
    }
    return "";
}

function vaelgVaeg(v) {
    vaeg = v;
    afsenderID = v + "WallWeb";
    espID = v + "WallESP";
    gem("WallsCIA-vaeg", v);
    for (let knap of vaegKnapper) {
        knap.remove();
    }
    vaegKnapper = [];
    hentHistorier();
    console.log("Denne side er " + VAEGGE[v].navn + " (" + afsenderID + ") med " + historier.length + " historier");
}


// ---------------- GEM I BROWSEREN ----------------

function hentGemt(noegle) {
    try {
        return window.localStorage.getItem(noegle);
    } catch (e) {
        console.log("Kan ikke læse fra browserens lager: " + e);
        return null;
    }
}

function gem(noegle, vaerdi) {
    try {
        window.localStorage.setItem(noegle, vaerdi);
    } catch (e) {
        console.log("Kan ikke gemme i browserens lager: " + e);
    }
}

// Hver væg har sin egen liste
function hentHistorier() {
    try {
        historier = JSON.parse(hentGemt("WallsCIA-historier-" + vaeg)) || [];
    } catch (e) {
        historier = [];
    }
}

function gemHistorier() {
    gem("WallsCIA-historier-" + vaeg, JSON.stringify(historier));
}


// ---------------- BESKEDER ----------------

// Bliver kaldt fra receiver.js, hver gang der kommer et knaptryk.
// Knaptryk håndteres her (og ikke i draw), så de kun udføres én gang pr. tryk
// og også virker, når browserfanen ligger i baggrunden.
function beskedModtaget() {
    // Vi lytter kun til vores egen vægs ESP - de andre vægges knapper ignoreres
    if (vaeg != "" && afsender == espID)  {
        if (val4 == 1)  {
            //knap 1 (pin 21) er trykket - læser en tilfældig historie højt
            afspilHistorie();
        }
        if (val3 == 1)  {
            //knap 2 (pin 22) er trykket - begynder at optage historie
            startOptagelse();
        }
        if (val2 == 1)  {
            //knap 3 (pin 23) er trykket - stopper optagelse og sender historien
            stopOptagelse();
        }
    }
}

// Bliver kaldt fra receiver.js, hver gang en anden væg sender en historie
function historieModtaget(tekst) {
    if (vaeg == "") {
        return; // vi ved ikke hvilken væg vi er endnu
    }
    // Kun historier fra de andre vægge gemmes (vores egne er sorteret fra i receiver.js)
    let kendtVaeg = false;
    for (let v in VAEGGE) {
        if (afsender == v + "WallWeb") {
            kendtVaeg = true;
        }
    }
    if (!kendtVaeg || typeof tekst != "string" || tekst.trim() == "") {
        console.log("Ignorerer historie, der ikke kommer fra en af væggene");
        return;
    }
    historier.push({ tekst: tekst.trim(), fra: afsender, tid: Date.now() });
    gemHistorier();
    visBesked("Ny historie modtaget");
    console.log("Historie gemt fra " + afsender + ". Listen har nu " + historier.length + " historier");
}

// Viser en kort besked på skærmen i 4 sekunder
function visBesked(tekst) {
    beskedTekst = tekst;
    beskedSlut = millis() + 4000;
}


// ---------------- SKÆRMEN ----------------

function draw() {
    background(220);
    fill(0);

    if (vaeg == "") {
        text("Vælg hvilken væg denne computer sidder i (knapperne under feltet)", 20, 20, width - 40, height - 40);
        return;
    }

    // Øverst: væggens farve, navn og antal historier
    fill(VAEGGE[vaeg].farve);
    rect(0, 0, width, 50);
    fill(0);
    text(VAEGGE[vaeg].navn + " - " + historier.length + " historier", 20, 34);

    if (optager) {
        text("Optager...\n" + transcript, 20, 70, width - 40, height - 120);
    } else if (window.speechSynthesis.speaking) {
        text("Afspiller...\n" + aktuelHistorie, 20, 70, width - 40, height - 120);
    } else if (millis() < beskedSlut) {
        text(beskedTekst, 20, 70, width - 40, height - 120);
    }

    // Browseren må først afspille lyd, når nogen har klikket på siden
    if (navigator.userActivation && !navigator.userActivation.hasBeenActive) {
        text("Klik på siden for at aktivere lyd", 20, height - 20);
    }
}

// Tastaturgenveje til opsætning:  x = slet væggens historier,  v = vælg væg forfra
function keyPressed() {
    if (vaeg == "") {
        return;
    }
    if (key == "x" || key == "X") {
        if (window.confirm("Slet alle " + historier.length + " historier på " + VAEGGE[vaeg].navn + "?")) {
            historier = [];
            sidstAfspillet = -1;
            gemHistorier();
            visBesked("Listen er tømt");
        }
    }
    if (key == "v" || key == "V") {
        if (window.confirm("Vælg en anden væg til denne computer?")) {
            try {
                window.localStorage.removeItem("WallsCIA-vaeg");
            } catch (e) {}
            window.location.href = window.location.pathname; // genindlæs uden ?wall=
        }
    }
}


// ---------------- AFSPILNING ----------------

// Læser en tilfældig historie fra listen højt med en dansk stemme
function afspilHistorie() {
    if (optager) {
        console.log("Kan ikke afspille, mens der optages");
        return;
    }
    if (historier.length == 0) {
        console.log("Der er ingen historier at afspille endnu");
        visBesked("Ingen historier endnu");
        return;
    }
    // En knap kan "prelle" og sende flere beskeder for ét tryk. Hvis vi starter forfra
    // for hver besked, hakker stemmen - så vi ignorerer tryk, mens historien læses op.
    let synth = window.speechSynthesis;
    if (synth.speaking || synth.pending || millis() - sidsteAfspilning < 1000) {
        console.log("Afspiller allerede - trykket ignoreres");
        return;
    }
    sidsteAfspilning = millis();

    // Vælg en tilfældig historie - men ikke den samme to gange i træk
    let nr = floor(random(historier.length));
    while (historier.length > 1 && nr == sidstAfspillet) {
        nr = floor(random(historier.length));
    }
    sidstAfspillet = nr;
    aktuelHistorie = historier[nr].tekst;
    console.log("Afspiller historie " + (nr + 1) + " af " + historier.length + " (fra " + historier[nr].fra + ")");

    let valg = findVaeggensStemme();
    if (valg.stemme) {
        mySpeech.utterance.voice = valg.stemme;
        console.log("Afspiller med stemmen: " + valg.stemme.name + " (tonehøjde " + valg.tonehoejde + ")");
    } else {
        console.log("Ingen dansk stemme fundet - bruger browserens standardstemme");
    }
    mySpeech.utterance.pitch = valg.tonehoejde;
    mySpeech.utterance.lang = "da-DK";
    mySpeech.speak(aktuelHistorie);
}

// Finder denne vægs stemme blandt browserens danske stemmer.
// Stemmerne er forskellige fra computer til computer, så vi vælger efter sprog og
// navn og ikke efter et fast nummer. Kun danske stemmer kan blive valgt.
function findVaeggensStemme() {
    let danske = window.speechSynthesis.getVoices().filter(s => s.lang.toLowerCase().startsWith("da"));

    // Gå væggens ønskeliste igennem og brug den første stemme, der findes
    for (let oenske of VAEGGE[vaeg].stemmer) {
        let fundet = danske.find(s => s.name.includes(oenske.navn));
        if (fundet) {
            return { stemme: fundet, tonehoejde: oenske.tonehoejde };
        }
    }

    // Ingen af væggens stemmer findes her: brug en anden dansk stemme
    // (helst en af de naturlige stemmer i Edge)
    console.log("Væggens egen stemme findes ikke på denne computer - bruger en anden dansk stemme");
    let naturlig = danske.find(s => s.name.includes("Natural"));
    return { stemme: naturlig || danske[0], tonehoejde: 1 };
}


// ---------------- OPTAGELSE ----------------

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
        // Historien sendes til de andre vægge. Vi gemmer den IKKE selv.
        sendHistorie(transcript.trim());
        visBesked("Historien er sendt til de andre vægge");
        console.log("Historie sendt: " + transcript.trim());
    } else {
        visBesked("Der blev ikke optaget noget");
    }
    transcript = "";
}

function optagelseFejl(fejl) {
    console.log("Fejl i talegenkendelse: " + fejl.error);
    if (fejl.error == "not-allowed" || fejl.error == "service-not-allowed" || fejl.error == "audio-capture") {
        // Ingen adgang til mikrofonen - stop, så vi ikke prøver igen og igen
        optager = false;
        visBesked("Ingen adgang til mikrofonen");
    }
}

function gotSpeech() {
    if (speechRec.resultValue) {
        transcript += speechRec.resultString + " "; // læg til i stedet for at overskrive
        console.log(transcript);                    // hele teksten kan også ses i konsollen
    }
}
