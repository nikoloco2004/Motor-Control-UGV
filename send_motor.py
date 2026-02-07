import serial
import time

PORT = "COM3"
BAUD = 115200

with serial.Serial(PORT, BAUD, timeout=0.2) as ser:
    time.sleep(4.0)          # Teensy can reset when serial opens

    ser.write(b"M 150\n")    # forward
    time.sleep(1.5)

    ser.write(b"M 0\n")      # stop
    time.sleep(0.8)

    ser.write(b"M -150\n")   # backward
    time.sleep(1.5)

    ser.write(b"M 0\n")      # stop
