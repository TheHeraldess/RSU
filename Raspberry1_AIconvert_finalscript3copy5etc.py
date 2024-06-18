import RPi.GPIO as GPIO
import time
from gpiozero import MCP3008
import smbus

# Konfiguration der GPIO-Pins
WATER_SENSOR = MCP3008(channel=3)
PUMPE = 4
BUTTON = 27

# LCD-Konfiguration
bus = smbus.SMBus(1)
DISPLAY_RGB_ADDR = 0x62
DISPLAY_TEXT_ADDR = 0x3e

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

def loop():
    while True:
        intensitaet2 = WATER_SENSOR.value * 1023  # MCP3008 gibt Werte von 0 bis 1 zurück
        intensitaet = round((10 * intensitaet2) / 1023, 2)
        regen = False

        time.sleep(2)

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
            time.sleep(1)
        else:
            setRGB(0, 255, 0)
            setText("Regen\nJa")
            time.sleep(4)
            GPIO.output(PUMPE, GPIO.HIGH)
            time.sleep(15)
            GPIO.output(PUMPE, GPIO.LOW)

if __name__ == '__main__':
    try:
        setup()
        loop()
    except KeyboardInterrupt:
        GPIO.cleanup()
