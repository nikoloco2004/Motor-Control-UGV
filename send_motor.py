import serial
import time

PORT = "COM13"
BAUD = 115200

def send(ser, cmd: str):
    # Always send newline-terminated ASCII commands
    ser.write((cmd.strip() + "\n").encode("ascii"))
    time.sleep(0.05)  # small gap so Teensy doesn't get spammed

with serial.Serial(PORT, BAUD, timeout=0.2) as ser:
    time.sleep(4.0)  # Teensy can reset when serial opens

    # --- Example sequence using the new protocol: M1..M4 value (-255..255) ---

    # All forward
    send(ser, "M1 255")
    send(ser, "M2 255")
    send(ser, "M3 255")
    send(ser, "M4 255")
    time.sleep(4.0)

    # Stop all
    send(ser, "M1 0")
    send(ser, "M2 0")
    send(ser, "M3 0")
    send(ser, "M4 0")
    time.sleep(0.8)

    # All backward (slower)
    send(ser, "M1 -150")
    send(ser, "M2 -150")
    send(ser, "M3 -150")
    send(ser, "M4 -150")
    time.sleep(1.5)

    # Stop all
    send(ser, "M1 0")
    send(ser, "M2 0")
    send(ser, "M3 0")
    send(ser, "M4 0")
    time.sleep(0.8)
    