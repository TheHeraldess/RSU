#include <Wire.h>
#include <rgb_lcd.h>
#include <avr/pgmspace.h>
#include <SPI.h>
#include <Ethernet.h>
#include <MLX90615.h>
#include <I2cMaster.h>

#define SDA_PIN 3
#define SCL_PIN 2 
#define WATER_SENSOR A3
#define pumpe 4
#define BUTTON 7

rgb_lcd lcd;

SoftI2cMaster i2c(SDA_PIN, SCL_PIN);
MLX90615 mlx90615(DEVICE_ADDR, &i2c);

unsigned long previousMillis = 0;
const long interval = 4000; // Interval für LCD-Updates
unsigned long pumpStartTime = 0; // Variable, um den Startzeitpunkt der Pumpe zu speichern
const long pumpDuration = 15000; // Dauer, für die die Pumpe aktiv sein soll
unsigned long letzteRegen = 0;
unsigned long startMillis = 0;
unsigned long wielang = 0;

int colorR = 0;
int colorG = 0;
int colorB = 255;

bool minuten = false;


byte mac[] = { 
 0x2C, 0xF7, 0xF1, 0x08, 0x1A, 0x72 };

const char code1[] PROGMEM = "<!DOCTYPE html><html lang='de'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>Regenradar</title><link rel='preconnect' href='https://fonts.googleapis.com'><link rel='preconnect' href='https://fonts.gstatic.com' crossorigin><link href='https://fonts.googleapis.com/css2?family=Open+Sans:ital,wght@0,500;1,500&display=swap' rel='stylesheet'><style>.headline {color: black;}.image-container {width: 100%;display: flex;justify-content: center;align-items: center;margin: 0 auto;max-width: fit-content;}.image {width: 45%;height: auto;margin: 0 2%;}.body {font-family: 'Open Sans', sans-serif;background-color: #F3F5F6;padding-left: 300px;padding-right: 300px;}.container {background-color: rgb(255, 255, 255);padding: 8px;}.navbar {display: flex;justify-content: space-between;align-items: center;background-color: aliceblue;}a {color: #831D17;text-decoration: none;margin-left: 16px;font-weight: 700;}a:hover {text-decoration: underline;}.wetter {font-weight: 800;background-color: aliceblue;}script {background-color: aliceblue;font-weight: 700;}</style></head><body><div class='roter-balken'></div><div class='navbar'><h1 class='headline'>Regenpausen-<br> Radar</h1><div><a href='https://www.wetter.com/deutschland/bitburg/DE0001210.html'>Wetter in Bitburg</a><a href='https://st-willi.de'>Schul Hompage</a><a href='https://makeyourschool.de'>Makeyourschool</a></div></div><div class='container'><div class='image-container'><img class='image' src='http://easy.box/dsl_usb1/BILDER/IMG1.JPG'><img class='image' src='";
const char link1[] PROGMEM = "http://easy.box/dsl_usb1/BILDER/IMG2.JPG";
const char link2[] PROGMEM = "http://easy.box/dsl_usb1/BILDER/IMG3.JPG";
char code2[] = "'></div><div id='wetter-ausgabe'>Aktuelle Temperatur: <b>";
char code3[] = "°C</b>, Wetter: <b>";
char code5 [] = "</b> letzter Regen vor: <b>";
char code4[] = "</b></div></div></body></html>";

bool regenETH = false;

IPAddress ip(192,168,2,101);
EthernetServer server(80);

void setup() {
  Serial.begin(9600);
  pinMode(WATER_SENSOR, INPUT);
  pinMode(pumpe, OUTPUT);
  pinMode(BUTTON, INPUT);
  lcd.begin(16, 2);
  lcd.setRGB(colorR, colorG, colorB);

  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("Server gestartet. IP: ");

  Serial.println(Ethernet.localIP());  

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
  startMillis = millis();
}

void loop() {
  unsigned long currentMillis = millis();
  float temperatur = mlx90615.getTemperature(MLX90615_AMBIENT_TEMPERATURE);
  int intensitaet2 = analogRead(WATER_SENSOR);
  int intensitaet = map(intensitaet2, 0, 1023, 10, 0);
  bool regen;
  bool regenETH2 = regenETH;

  if(intensitaet < 8){
    regenETH = false;
  }else{
    regenETH = true;
  }
  if(regenETH2 == false && regenETH == true){
    if(letzteRegen == 0){
      letzteRegen = currentMillis - startMillis;
    }else{
      letzteRegen = currentMillis;
    }
  }
  wielang = currentMillis - letzteRegen;
  wielang = wielang / 1000;
  if (wielang > 120){
    wielang = wielang / 60;
    minuten = true;
  }else{
    minuten = false;
  }

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

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
  EthernetClient client = server.available();
  if (client) {
    Serial.println("Neuer Client");
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
      char c = client.read();
      Serial.write(c);
      if (c == '\n' && currentLineIsBlank) {
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println("Connection: close"); // Verbindung wird nach Antwort beendet
        client.println("Refresh: 5"); // Seite alle 25 Sekunden neu abfragen
        client.println();
        for (int i = 0; i < strlen_P(code1); i++) {
          client.write(pgm_read_byte_near(code1 + i));
        }
        if (regenETH == false) {
          for (int i = 0; i < strlen_P(link1); i++) {
            client.write(pgm_read_byte_near(link1 + i));
          }
        }else{
          for (int i = 0; i < strlen_P(link2); i++) {
            client.write(pgm_read_byte_near(link2 + i));
          }
        }
        client.print(code2);
        client.print(temperatur);
        client.print(code3);
        if (regenETH == false){
          client.print("KEIN REGEN, ");
        }else{
          client.print("ES REGNET, ");
        }
        client.print(code5);
        if (letzteRegen == 0){
          client.print("--");
        }else{
          client.print(wielang);
          if(minuten == HIGH){
            client.print("min");
          }else{
            client.print("s");
          }
        }
        client.print(code4);
        break;
      }
      if (c == '\n') {
        currentLineIsBlank = true;
      }   
      else if (c != '\r') {
        currentLineIsBlank = false;
      }
    }
  }


  client.stop();
  Serial.println("Verbindung mit Client beendet.");
  Serial.println("");
 }  
}

