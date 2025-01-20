#Comments by ChatGPT
#Hier ist der angepasste Python-Code, der die gesammelten Daten in einer JSON-Datei speichert. Die Kommentare im Code und die Erklärungen am Anfang sollen dir helfen, die Funktionsweise besser zu verstehen.

#Erklärungen:

#MLX90614: Verwendet zur Messung der Temperatur.
#GPIO: Konfiguriert die GPIO-Pins für die Pumpe, den Wassersensor und den Button.
#LCD: Verwendet zur Anzeige von Informationen.
#JSON-Datei: Speichert die Wetterdaten.


import RPi.GPIO as GPIO
import time
from gpiozero import MCP3008
import smbus
from mlx90614 import MLX90614
import json

# GPIO-Konfiguration
SDA_PIN = 2
SCL_PIN = 3
WATER_SENSOR = MCP3008(channel=3)
PUMPE = 4
BUTTON = 27

# LCD-Konfiguration
bus = smbus.SMBus(1)
DISPLAY_RGB_ADDR = 0x62
DISPLAY_TEXT_ADDR = 0x3e

# MLX90614-Konfiguration
mlx = MLX90614(bus)

# GPIO-Modus festlegen
GPIO.setmode(GPIO.BCM)
GPIO.setup(PUMPE, GPIO.OUT)
GPIO.setup(BUTTON, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

# Datei, in der die Daten gespeichert werden
data_file = 'wetterdaten.json'

# Hilfsfunktionen für LCD
def setRGB(r, g, b):
    bus.write_byte_data(DISPLAY_RGB_ADDR, 0, 0)
    bus.write_byte_data(DISPLAY_RGB_ADDR, 1, 0)
    bus.write_byte_data(DISPLAY_RGB_ADDR, 0x08, 0xaa)
    bus.write_byte_data(DISPLAY_RGB_ADDR, 4, r)
    bus.write_byte_data(DISPLAY_RGB_ADDR, 3, g)
    bus.write_byte_data(DISPLAY_RGB_ADDR, 2, b)

def textCommand(cmd):
    bus.write_byte_data(DISPLAY_TEXT_ADDR, 0x80, cmd)

def setText(text):
    textCommand(0x01)
    time.sleep(0.05)
    textCommand(0x08 | 0x04)
    textCommand(0x28)
    time.sleep(0.05)
    count = 0
    row = 0
    for c in text:
        if c == '\n' or count == 16:
            count = 0
            row += 1
            if row == 2:
                break
            textCommand(0xc0)
            if c == '\n':
                continue
        count += 1
        bus.write_byte_data(DISPLAY_TEXT_ADDR, 0x40, ord(c))

def setup():
    setRGB(0, 0, 255)
    setText("Starte\nSystem")
    time.sleep(4)
    setText("Ethernet\nVerbunden")
    time.sleep(4)

def get_sensor_data():
    temperatur = mlx.get_ambient()
    intensitaet2 = WATER_SENSOR.value * 1023  # MCP3008 gibt Werte von 0 bis 1 zurück
    intensitaet = round((10 * intensitaet2) / 1023, 2)
    return temperatur, intensitaet

def save_to_json(data):
    with open(data_file, 'w') as outfile:
        json.dump(data, outfile, indent=4)

def update_lcd_and_pump(regen, intensitaet, currentMillis):
    global pumpStartTime

    if intensitaet < 8:
        regen = False
        print("Kein Regen")
    else:
        regen = True
        print("Es regnet")

    if GPIO.input(BUTTON) == GPIO.HIGH:
        regen = True

    if not regen:
        setRGB(255, 0, 0)
        setText("Regen\nNein")
        GPIO.output(PUMPE, GPIO.LOW)  # Stelle sicher, dass die Pumpe ausgeschaltet ist
    else:
        setRGB(0, 255, 0)
        setText("Regen\nJa")
        if pumpStartTime == 0:  # Wenn die Pumpe noch nicht gestartet wurde
            pumpStartTime = currentMillis  # Speichere den Startzeitpunkt der Pumpe
            GPIO.output(PUMPE, GPIO.HIGH)  # Starte die Pumpe
        else:
            if currentMillis - pumpStartTime >= 15000:  # Überprüfe, ob die Pumpe lange genug gelaufen ist
                GPIO.output(PUMPE, GPIO.LOW)  # Stoppe die Pumpe
                pumpStartTime = 0  # Setze den Startzeitpunkt zurück

def loop():
    previousMillis = 0
    interval = 4000  # Interval für LCD-Updates
    pumpStartTime = 0

    while True:
        currentMillis = time.time() * 1000
        temperatur, intensitaet = get_sensor_data()

        if currentMillis - previousMillis >= interval:
            previousMillis = currentMillis

            regen = False
            update_lcd_and_pump(regen, intensitaet, currentMillis)

            data = {
                "temperatur": temperatur,
                "regen": regen,
                "intensitaet": intensitaet,
                "timestamp": time.strftime("%Y-%m-%d %H:%M:%S")
            }

            save_to_json(data)

if __name__ == '__main__':
    try:
        setup()
        loop()
    except KeyboardInterrupt:
        GPIO.cleanup()



#Comments by ChatGPT:
#Wichtige Punkte im Code:

#Initialisierung und Setup: setup() initialisiert das LCD und zeigt Startmeldungen an.
#Sensordaten: get_sensor_data() liest die Temperatur und Intensität vom Wassersensor.
#Daten speichern: save_to_json(data) speichert die gesammelten Daten in einer JSON-Datei.
#Aktualisierung von LCD und Pumpe: update_lcd_and_pump(regen, intensitaet, currentMillis) aktualisiert den LCD-Bildschirm und steuert die Pumpe basierend auf dem Regenerkennung und der gemessenen Intensität.
#Hauptschleife: loop() führt die Hauptschleife aus, die die Sensordaten liest, das LCD und die Pumpe aktualisiert und die Daten in einer JSON-Datei speichert.

#Die JSON-Datei enthält die Temperatur, den Regenerkennungsstatus, die Intensität und den Zeitstempel der Messung.