#include <SPI.h>        //ETH
#include <Ethernet.h>   //ETH
#include <Wire.h>       //STRG RSU
#include <rgb_lcd.h>    //STRG RSU
#include <MLX90615.h>   //Tempsensor
#include <I2cMaster.h>  //Tempsensor
#define WATER_SENSOR A3  //Zahl = Port , #definie NAME_NAME = Senor definierten (Alles groß!)
#define PUMPE 4          //Wichtig
#define BUTTON 7
#define SDA_PIN 3
#define SCL_PIN 2 

//ETH
byte mac[] = { 0x2C, 0xF7, 0xF1, 0x08, 0x1A, 0x72 };

IPAddress ip(192,168,2,145);
IPAddress server2(192, 168, 2, 100);
IPAddress raspberry(192, 168, 2, 100);

EthernetServer server(80);


//ETH - LETZTE REGEN
bool regenETH = false;
bool regenETH2;
unsigned long letzteRegen = 0;
unsigned long startMillis = 0;
int wielangh = 0;
int wielangmin = 0;
int wielangs = 0;
int wielangd = 0;


//STRG
rgb_lcd lcd;


//STRG - milis
unsigned long previousMillis = 0;
unsigned long previousMillisETH = 0;
const long interval = 4000;
unsigned long pumpeStart = 0;
const long pumpeDauer = 15000;



//Temp
SoftI2cMaster i2c(SDA_PIN, SCL_PIN);
MLX90615 mlx90615(DEVICE_ADDR, &i2c);

//DEBUG
bool wartungpumpe = false;




void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.begin(9600);
  pinMode(WATER_SENSOR, INPUT);
  pinMode(PUMPE, OUTPUT);
  pinMode(BUTTON, INPUT);
  pinMode(8, OUTPUT);
  //pinMode(lcd, OUTPUT)  
  lcd.begin(16, 2);
  lcd.setRGB(0, 0, 255);  

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
  //TEMP
  float temperatur = mlx90615.getTemperature(MLX90615_AMBIENT_TEMPERATURE);

  //STRG
  unsigned long currentMillis = millis();
  int intensitaet2 = analogRead(WATER_SENSOR);
  int intensitaet = map(intensitaet2, 0, 1023, 10, 0);
  bool regen; 
  

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
      if(!wartungpumpe){
        digitalWrite(PUMPE, LOW); // Stelle sicher, dass die Pumpe ausgeschaltet ist
      }
    }
    if (regen) {
      lcd.clear();
      lcd.setRGB(0, 255, 0);
      lcd.setCursor(0, 0);
      lcd.print("Regen");
      lcd.setCursor(0, 1);
      lcd.print("Ja");

      if (pumpeStart == 0) { // Wenn die Pumpe noch nicht läuft
        pumpeStart = currentMillis; 
        digitalWrite(PUMPE, HIGH);
      }

      // Solange Regen erkannt wird, bleibt die Pumpe an
    } else {
      // Prüfen, ob die Pumpe mindestens 15s lief, bevor sie gestoppt wird
      if (pumpeStart > 0 && currentMillis - pumpeStart >= pumpeDauer) {
        digitalWrite(PUMPE, LOW);
        pumpeStart = 0; // Jetzt zurücksetzen, damit sie beim nächsten Regen wieder starten kann
      }
    }
  }


  //DAUER SEIT LETZTEM REGEN / Dauer des Regens
  regenETH2 = regenETH;
   
  if(intensitaet < 8){
    regenETH = false;
  }else{
    regenETH = true;
  }

  if(!regenETH2 && regenETH || regenETH2 && !regenETH){
    if(letzteRegen == 0){
      letzteRegen = currentMillis - startMillis;
    }else{
      letzteRegen = currentMillis;
    }
  }

  unsigned long seitLetztemRegen = (currentMillis - letzteRegen) / 1000;
  wielangd = seitLetztemRegen / 86400;
  seitLetztemRegen %= 86400;
  wielangh = seitLetztemRegen / 3600;
  seitLetztemRegen %= 3600;
  wielangmin = seitLetztemRegen / 60;
  wielangs = seitLetztemRegen % 60;


  /*
  wielangs = currentMillis - letzteRegen;
  wielangs = wielangs / 1000;
  wielangmin = wielangs / 60;
  wielangh = wielangmin /60;
  wielangd = wielangh / 24;
  if(wielangmin > 0){
    wielangs = wielangs - wielangmin * 60;
  }
  if (wielangh > 0){
    wielangmin = wielangmin - wielangh * 60;
  }
  if (wielangd > 0){
    wielangh = wielangh - wielangd * 24;
  }*/

  //REQUEST-DATA
  String temprq = String(temperatur, 2);
  temprq.replace(".", "");
  String rqtemp;
  if(temperatur < 10){
    rqtemp = "0" + temprq; 
  }else if(temperatur >= 100){
    rqtemp = "0000";
  }else{
    rqtemp = temprq;
  }
  
  int regenrq;
  if(regenETH){
    regenrq = 1;
  }else{
    regenrq = 0;
  }
  String rqregen = String(regenrq);


  String rqs = String(wielangs);
  String rqmin = String(wielangmin);
  String rqh = String(wielangh);
  String rqd = String(wielangd);

  if(wielangs < 10){
    rqs = "0" + rqs;
  }

  if(wielangmin < 10){
    rqmin = "0" + rqmin;
  }

  if(wielangh < 10){
    rqh = "0" + rqh;
  }

  if(wielangd < 10){
    rqd = "0" + rqd;
  }
  if(wielangd > 99){
    rqd = rqd[rqd.length()] + rqd[rqd.length() + 1];
  }

  String rqlztrRg = rqd + rqh + rqmin + rqs;

  //ETH
  String data = rqregen + rqtemp + rqlztrRg;                       //"1270008564563" //Regen 0/1(1) - 2 zahlen temp.(2) - letzter regen / dauer des Regens in tagen(2) in stunden(2) in minuten(2) und in sekunden(2)

  if(currentMillis - previousMillisETH >= 5000){
    previousMillisETH = currentMillis;
    EthernetClient client2;
    if (client2.connect(server2, 80)) {
      client2.println("POST /empfang.html HTTP/1.1");
      client2.println("Host: 192.168.2.100"); // die IP des raspis
      client2.println("Content-Type: application/x-www-form-urlencoded");
      client2.println("Content-Length: 18"); // 13 für var + 5 für data=

      client2.println();
      client2.println("data=" + data);

      // Serverantwort lesen    UNNÖTIG
      while (client2.connected()) {
        if (client2.available()) {
          char c = client2.read();
          //Serial.print(c);
        }
      }

      // Verbindung schließen
      client2.stop();
    } else {
      Serial.println("Connection failed.");
    }
    Serial.println(data);
  }
  EthernetClient client = server.available();
  if(client) {
    Serial.println("client is da");
    String currentLine = "";
    String request = "";
    boolean currentLineIsBlank = true;
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      currentLine += c;

      // Ende der Request-Line
      if (c == '\n') {
        // Nur die erste Zeile interessiert uns (Request-Line)
        if (currentLine.startsWith("GET")) {
          request = currentLine;
          Serial.println("Request erhalten: " + request);
        }
        currentLine = ""; // Zeile zurücksetzen für nächsten Durchlauf
      }

      // Header-Ende erreicht
      if (currentLine.length() == 1 && currentLine[0] == '\r') {
        break;
      }
    }
  }
  if (client.remoteIP() == raspberry) {
    if (request.length() > 0) {
      int paramIndex = request.indexOf("?");
      int endIndex = request.indexOf(" ", paramIndex);
      if (paramIndex != -1) {
        String params = request.substring(paramIndex + 1, endIndex);
        aktionen(params);
      }
    }
  }
  

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head><title>Arduino POST</title><meta charset='UTF-8'></head>");
  client.println("<body><h1>Herzlich Wilkommen! Dies ist eine für Sie unnötige Seite </h1></body>");
  client.println("</html>");  
  // Schließt die Verbindung zum Client
  delay(1);
  client.stop();
  Serial.println("Client-Verbindung geschlossen");
  }
}


void aktionen(String parameter){
  char aktion = parameter[0];
  int aktascii = int(aktion);
  Serial.println(aktion);

  if(aktascii == 49){      // ASCII 0=48; 1=49; 2=50; 3=51 ...
    wartungpumpe = true;
    digitalWrite(PUMPE, HIGH);
    Serial.println("Hello from Aktionen");
  }else if (aktascii == 50){
    wartungpumpe = false;
    digitalWrite(PUMPE, LOW);
  }else if (aktascii == 51){
  }
}