const topic = "WallsCIA";
const afsenderID = "BlueWallWeb";



function setup() {
    createCanvas(200, 200);
    background(220);

}


function draw() {

    
    
    if (afsenderID == BlueWallESP)  {
        console.log("BlueWallESP is connected");
        if (val2 == 1)  {
        background(0, 255, 0);
        //så er første knap trykket og jeg afspiller en historie
        }
        if (val3 == 1)  {
        background(255, 0, 0);
        // begynder at optage historie
        }
        if (val4 == 1)  {
        //stopper optagelse og sender historie
        }
    }
    // skal nok have noget til at resette de andre values tilbage til 0, dette skal måske også sendes til ESPen???

    }
    

