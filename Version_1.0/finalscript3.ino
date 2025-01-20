
#define WATER_SENSOR A3  //Zahl = Port , #definie NAME_NAME = Senor definierten (Alles groß!)
#define pumpe 4          //Wichtig
#define BUTTON 3
#include <Wire.h>
#include "rgb_lcd.h"

rgb_lcd lcd;

int colorR = 0;
int colorG = 255;
int colorB = 0;



void setup() {
  Serial.begin(9600);
  pinMode(WATER_SENSOR, INPUT);
  pinMode(pumpe, OUTPUT);
  pinMode(BUTTON, INPUT);
  //pinMode(lcd, OUTPUT)
  lcd.begin(16, 2);
  lcd.setRGB(colorR, colorG, colorB);

  // Print a message to the LCD.
  lcd.print("Wasser");
  Serial.println("Test1");
}

void loop() {


  int intensitaet2 = analogRead(WATER_SENSOR);
  int intensitaet = map(intensitaet2, 0, 1023, 10, 0);
  bool regen;





  if (intensitaet < 8) {
    regen = false;
    Serial.println("Kein Regen");
  } else {
    regen = true;
    Serial.println("Es regnet");
  }


  if (digitalRead(BUTTON) == HIGH) {  (regen = true);
  }




  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  if (regen == false) {
    lcd.clear();
    int colorG = 255;
    int colorR = 0;
    lcd.setCursor(0, 0);
    lcd.print("Regen");
    lcd.setCursor(0, 1);
    lcd.print("Nein");
    


  } 
  if (regen == true) {
    lcd.clear();
    int colorG = 0;
    int colorR = 255;
    lcd.setCursor(0, 0);
    lcd.print ("Regen");
    lcd.setCursor(0, 1);
    // print the number of seconds since reset:
    lcd.print("Ja");
        delay(4000);  //Wartezeit
    digitalWrite(pumpe, HIGH);
    delay(15000);  //Wartezeit
    digitalWrite(pumpe, LOW);
  }
}
