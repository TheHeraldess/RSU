long randNumber;
//Random number settings

//read of empty pin recommended for random seed every restart
int RANDPIN = 1;
//area in which numbers are generated
int randomArea = "2103";
int randomMax = "3206";


void setup() {

   Serial.begin(9600);

  randomSeed(analogRead(RANDPIN));
}


void loop() {

   randNumber = random(randomArea, randomMax);
   Serial.println(randNumber);

   delay(200);
}
