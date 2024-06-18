#define WATER_SENSOR A3
#define pumpe 4
#define BUTTON 3
#include <Wire.h>
#include "rgb_lcd.h"
#include <SPI.h>
#include <Ethernet.h>
#include <avr/pgmspace.h>
#include <MLX90615.h>
#include <I2cMaster.h>
#include <SPI.h>
#include <Ethernet.h>

rgb_lcd lcd;

#define SDA_PIN 3
#define SCL_PIN 2 

SoftI2cMaster i2c(SDA_PIN, SCL_PIN);
MLX90615 mlx90615(DEVICE_ADDR, &i2c);

byte mac[] = { 
 0x2C, 0xF7, 0xF1, 0x08, 0x1A, 0x72 };


IPAddress ip(192,168,2,145);

EthernetServer server(80);

unsigned long previousMillis = 0;
const long interval = 4000; // Interval für LCD-Updates
unsigned long pumpStartTime = 0; // Variable, um den Startzeitpunkt der Pumpe zu speichern
const long pumpDuration = 15000; // Dauer, für die die Pumpe aktiv sein soll

const char code[] PROGMEM = "<!DOCTYPE html><html lang='de'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>Regenradar</title><link rel='preconnect' href='https://fonts.googleapis.com'><link rel='preconnect' href='https://fonts.gstatic.com' crossorigin><link href='https://fonts.googleapis.com/css2?family=Open+Sans:ital,wght@0,500;1,500&display=swap' rel='stylesheet'><style>.headline {color: black;}.image-container {width: 100%;display: flex;justify-content: center;align-items: center;margin: 0 auto;max-width: fit-content;}.image {width: 45%;height: auto;margin: 0 2%;}.body {font-family: 'Open Sans', sans-serif;background-color: #F3F5F6;padding-left: 300px;padding-right: 300px;}.container {background-color: rgb(255, 255, 255);padding: 8px;}.navbar {display: flex;justify-content: space-between;align-items: center;background-color: aliceblue;}a {color: #831D17;text-decoration: none;margin-left: 16px;font-weight: 700;}a:hover {text-decoration: underline;}.wetter {font-weight: 800;background-color: aliceblue;}script {background-color: aliceblue;font-weight: 700;}</style></head><body><div class='roter-balken'></div><div class='navbar'><h1 class='headline'>Regenpausen-<br> Radar</h1><div><a href='https://www.wetter.com/deutschland/bitburg/DE0001210.html'>Wetter in Bitburg</a><a href='https://st-willi.de'>Schul Hompage</a><a href='https://makeyourschool.de'>Makeyourschool</a></div></div><div class='container'><div class='image-container'><img class='image' src='https://media.licdn.com/dms/image/C5612AQFJPAGbnXOyGQ/article-cover_image-shrink_600_2000/0/1520042361358?e=2147483647&v=beta&t=doFrztYRczfC57fkpc2-8pBP9ULWShKTz4jc15GdVy8'><img class='image' src='";
const char link1[] PROGMEM = "https://www.rosenheim24.de/assets/images/7/639/7639277-1954074307-wetter-1j70.jpg";
const char link2[] PROGMEM = "https://images.ctfassets.net/4ivszygz9914/5AZCB8KFHqYgE0FFAH34A6/e52551ba7e0abe3517488bf90187e1ad/regen.jpg?fm=webp";
const char code2[] PROGMEM = "'></div><div id='wetter-ausgabe'>Aktuelle Temperatur: ";
const char code4[] PROGMEM = "</div></div></body></html>";

char code3[] = "°C, Wetter: ";
char temp[] = "11";

bool regen2;
bool regen;
bool regen5 = true;


int colorR = 0;
int colorG = 0;
int colorB = 255;

void setup() {
  Serial.begin(9600);
  pinMode(WATER_SENSOR, INPUT);
  pinMode(pumpe, OUTPUT);
  pinMode(BUTTON, INPUT);
 Ethernet.begin(mac, ip);
 server.begin();
 Serial.print("Server gestartet. IP: ");
 // IP des Arduino-Servers ausgeben
 Serial.println(Ethernet.localIP());


  lcd.begin(16, 2);
  lcd.setRGB(colorR, colorG, colorB);

  Serial.println("Test1");
  lcd.print("Starte");
  lcd.setCursor(0, 1);
  lcd.print("System");
  delay(4000);
  lcd.setCursor(0, 0);
  lcd.print("Ethernet");
  lcd.setCursor(0, 1);
  lcd.print("Verbunden");
  delay(4000);
}

void sendProgmemString(EthernetClient client, const char* str) {
  char c;
  while (c = pgm_read_byte(str++)) {
    client.write(c);
  }
}


void loop() {
  unsigned long currentMillis = millis();
  float temperatur = mlx90615.getTemperature(MLX90615_AMBIENT_TEMPERATURE);

  int intensitaet2 = analogRead(WATER_SENSOR);
  int intensitaet = map(intensitaet2, 0, 1023, 10, 0);

  if (intensitaet < 8) {
    regen = false;
    Serial.println("Kein Regen");
  } else {
    regen = true;
    Serial.println("Es regnet");
  }

  if (digitalRead(BUTTON) == HIGH) {
    regen = true;
  }

 EthernetClient client = server.available();
 // Wenn es einen Client gibt, dann...
 if (client) {
 Serial.println("Neuer Client");
 // Jetzt solange Zeichen lesen, bis eine leere Zeile empfangen wurde
 // HTTP Requests enden immer mit einer leeren Zeile 
 boolean currentLineIsBlank = true;
 // Solange Client verbunden 
 while (client.connected()) {
 // client.available() gibt die Anzahl der Zeichen zurück, die zum Lesen
 // verfügbar sind
 if (client.available()) {
 // Ein Zeichen lesen und am seriellen Monitor ausgeben
 char c = client.read();
 Serial.write(c);
  
 if (c == '\n' && currentLineIsBlank) {
 // HTTP Header 200 an den Browser schicken
 client.println("HTTP/1.1 200 OK");
 client.println("Content-Type: text/html");
 client.println("Connection: close"); // Verbindung wird nach Antwort beendet
 client.println("Refresh: 5"); // Seite alle 25 Sekunden neu abfragen
 client.println();
 // Ab hier berginnt der HTML-Code, der an den Browser geschickt wird
 
 if(intensitaet < 8){
 sendProgmemString(client, code);
 sendProgmemString(client, link1);
 sendProgmemString(client, code2);
 client.print(temperatur);
 client.print(code3);
 client.print("Kein Regen");
 sendProgmemString(client, code4);
 }else{
 sendProgmemString(client, code);
 sendProgmemString(client, link2);
 sendProgmemString(client, code2);
 client.print(temperatur);
 client.print(code3);
 client.print("Es regnet");
 sendProgmemString(client, code4);
 }

 break;
 }
 if (c == '\n') {
 // Zeilenwechsel, also currentLineIsBlack erstmal auf True setzen
 currentLineIsBlank = true;
 } 
 else if (c != '\r') {
 // Zeile enthält Zeichen, also currentLineIsBlack auf False setzen
 currentLineIsBlank = false;
 }
 }
 }
 // Kleine Pause
 delay(1);
 // Verbindung schliessen
 client.stop();
 Serial.println("Verbindung mit Client beendet.");
 Serial.println("");
 }

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    if (regen == false) {
      lcd.clear();
      const int colorR = 255;
      const int colorG = 0;
      const int colorB = 0;
      lcd.setRGB(colorR, colorG, colorB);
      lcd.setCursor(0, 0);
      lcd.print("Regen");
      lcd.setCursor(0, 1);
      lcd.print("Nein");
      digitalWrite(pumpe, LOW); // Stelle sicher, dass die Pumpe ausgeschaltet ist
    }
    if (regen == true) {
      lcd.clear();
      int colorR = 0;
      int colorG = 255;
      int colorB = 0;
      lcd.setRGB(colorR, colorG, colorB);
      lcd.setCursor(0, 0);
      lcd.print("Regen");
      lcd.setCursor(0, 1);
      lcd.print("Ja");

      if (pumpStartTime == 0) { // Wenn die Pumpe noch nicht gestartet wurde
        pumpStartTime = currentMillis; // Speichere den Startzeitpunkt der Pumpe
        digitalWrite(pumpe, HIGH); // Starte die Pumpe
      } else {
        if (currentMillis - pumpStartTime >= pumpDuration) { // Überprüfe, ob die Pumpe lange genug gelaufen ist
          digitalWrite(pumpe, LOW); // Stoppe die Pumpe
          pumpStartTime = 0; // Setze den Startzeitpunkt zurück
        }
      }
    }
  }
}


