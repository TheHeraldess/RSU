/*
 Web Server
 
 Eine einfacher Webserver, der den Messwert auf Analogpin 0 als HTML zurückgibt.

 Notwendig sind:
 - Arduino Uno R3
 - Arduino Ethernet Shield
 - Analoger Sensor (z.B. lichtempfindlicher Widerstand an A0)
 
Based on script by David A. Mellis and Tom Igoe
 */

// Beide Libraries sind erforderlich
#include <SPI.h>
#include <Ethernet.h>

// Hier die MAC Adresse des Shields eingeben
// (Aufkleber auf Rückseite)
byte mac[] = { 
 0x2C, 0xF7, 0xF1, 0x08, 0x1A, 0x72 };

// Eine IP im lokalen Netzwerk angeben. Dazu am besten die IP
// des PCs herausfinden (googlen!) und die letzte Zahl abändern 
IPAddress ip(192,168,2,37);

// Ethernet Library als Server initialisieren
// Verwendet die obige IP, Port ist per default 80
EthernetServer server(80);

void setup() {
 // Serielle Kommunikation starten, damit wir auf dem Seriellen Monitor
 // die Debug-Ausgaben mitlesen können.
 Serial.begin(9600);

 // Ethernet Verbindung und Server starten
 Ethernet.begin(mac, ip);
 server.begin();
 Serial.print("Server gestartet. IP: ");
 // IP des Arduino-Servers ausgeben
 Serial.println(Ethernet.localIP());
}


void loop() {
 // server.available() schaut, ob ein Client verfügbar ist und Daten
 // an den Server schicken möchte. Gibt dann eine Client-Objekt zurück,
 // sonst false
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
 // In currentLineIsBlank merken wir uns, ob diese Zeile bisher leer war.
 // Wenn die Zeile leer ist und ein Zeilenwechsel (das \n) kommt,
 // dann ist die Anfrage zu Ende und wir können antworten
 if (c == '\n' && currentLineIsBlank) {
 // HTTP Header 200 an den Browser schicken
 client.println("HTTP/1.1 200 OK");
 client.println("Content-Type: text/html");
 client.println("Connection: close"); // Verbindung wird nach Antwort beendet
 client.println("Refresh: 2"); // Seite alle 25 Sekunden neu abfragen
 client.println();
 // Ab hier berginnt der HTML-Code, der an den Browser geschickt wird
 client.println("<!DOCTYPE HTML>");
 client.println("<html>");
 client.print("Analogpin 0: <b>");
 client.print(analogRead(A0));
 client.println("</b><br />"); 
 client.println("</html>");
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
}