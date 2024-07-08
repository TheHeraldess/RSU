#Comments by ChatGPT
#Hier ist der umgewandelte Python-Code für den Raspberry Pi unter Nutzung der GPIO-Pins und der entsprechenden Bibliotheken. Einige Arduino-spezifische Bibliotheken wie SoftI2cMaster, MLX90615, und Ethernet müssen durch Raspberry Pi-kompatible Alternativen ersetzt werden.

#Mapping der Pins:

#SDA_PIN (3) → GPIO 2 (BCM Mode, SDA)
#SCL_PIN (2) → GPIO 3 (BCM Mode, SCL)
#WATER_SENSOR (A3) → GPIO 17 (BCM Mode, analog über MCP3008)
#pumpe (4) → GPIO 4
#BUTTON (7) → GPIO 27



import RPi.GPIO as GPIO
import time
from gpiozero import MCP3008
import smbus
from mlx90614 import MLX90614
import socket
from http.server import BaseHTTPRequestHandler, HTTPServer

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

# MLX90615-Konfiguration
mlx = MLX90614(bus)

# Ethernet-Konfiguration
server_port = 80
ip = "192.168.2.101"

# GPIO-Modus festlegen
GPIO.setmode(GPIO.BCM)
GPIO.setup(PUMPE, GPIO.OUT)
GPIO.setup(BUTTON, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

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

def update_lcd_and_pump(regen, intensitaet):
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
        GPIO.output(PUMPE, GPIO.HIGH)
        time.sleep(15)
        GPIO.output(PUMPE, GPIO.LOW)

class MyHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        temperatur, intensitaet = get_sensor_data()
        if intensitaet < 8:
            regenETH = False
        else:
            regenETH = True

        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()

        response = "<!DOCTYPE html><html lang='de'><head><meta charset='UTF-8'>"
        response += "<meta name='viewport' content='width=device-width, initial-scale=1.0'><title>Regenradar</title>"
        response += "<link rel='stylesheet' href='https://fonts.googleapis.com/css2?family=Open+Sans:wght@500&display=swap'>"
        response += "<style>.headline {color: black;} .image-container {width: 100%; display: flex; justify-content: center; align-items: center; margin: 0 auto; max-width: fit-content;}"
        response += ".image {width: 45%; height: auto; margin: 0 2%;} .body {font-family: 'Open Sans', sans-serif; background-color: #F3F5F6; padding-left: 300px; padding-right: 300px;}"
        response += ".container {background-color: rgb(255, 255, 255); padding: 8px;} .navbar {display: flex; justify-content: space-between; align-items: center; background-color: aliceblue;} a {color: #831D17; text-decoration: none; margin-left: 16px; font-weight: 700;}"
        response += "a:hover {text-decoration: underline;} .wetter {font-weight: 800; background-color: aliceblue;} script {background-color: aliceblue; font-weight: 700;}</style></head>"
        response += "<body><div class='roter-balken'></div><div class='navbar'><h1 class='headline'>Regenpausen-<br> Radar</h1><div>"
        response += "<a href='https://www.wetter.com/deutschland/bitburg/DE0001210.html'>Wetter in Bitburg</a>"
        response += "<a href='https://st-willi.de'>Schul Homepage</a><a href='https://makeyourschool.de'>Makeyourschool</a></div></div>"
        response += "<div class='container'><div class='image-container'><img class='image' src='http://easy.box/dsl_usb1/BILDER/IMG1.JPG'><img class='image' src='"
        response += "http://easy.box/dsl_usb1/BILDER/IMG2.JPG" if not regenETH else "http://easy.box/dsl_usb1/BILDER/IMG3.JPG"
        response += "'></div><div id='wetter-ausgabe'>Aktuelle Temperatur: <b>{:.2f}°C</b>, Wetter: <b>{}</b>".format(temperatur, "KEIN REGEN" if not regenETH else "ES REGNET")
        response += "</div></div></body></html>"

        self.wfile.write(response.encode())

def run_server():
    server_address = (ip, server_port)
    httpd = HTTPServer(server_address, MyHandler)
    print(f"Server gestartet. IP: {ip}")
    httpd.serve_forever()

if __name__ == '__main__':
    try:
        setup()
        run_server()
    except KeyboardInterrupt:
        GPIO.cleanup()





#Extra Comments
#Hinweise und Erläuterungen:

#MLX90614: Anstelle von MLX90615 habe ich MLX90614 verwendet, da es kompatible Bibliotheken für den Raspberry Pi gibt.
#LCD-Steuerung: Die I2C-Kommunikation mit dem LCD wird mit smbus realisiert.
#Ethernet: Anstelle von Ethernet wird ein einfacher HTTP-Server mit http.server genutzt.
#GPIO: Die GPIO-Pins werden entsprechend konfiguriert.
#Dieser Code sollte auf einem Raspberry Pi laufen und die gleichen Funktionen wie der ursprüngliche Arduino-Code ausführen.