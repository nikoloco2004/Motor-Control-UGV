import serial
import time

PORT = "COM13"
BAUD = 115200
SPEED = 180

def write_line(ser, s: str):
    ser.write((s.strip() + "\n").encode("ascii"))

def drain(ser, seconds=0.6):
    end = time.time() + seconds
    while time.time() < end:
        line = ser.readline().decode(errors="ignore").strip()
        if line:
            print("   [Teensy]", line)

with serial.Serial(PORT, BAUD, timeout=0.2) as ser:
    print("Opened serial, waiting for Teensy reset...")
    time.sleep(4.0)
    drain(ser, 1.5)

    for m in [1, 2, 3, 4]:
        input(f"\nPress ENTER to test M{m} (wheel should spin NOW if it's wired to M{m})...")
        print(f">>> SELECT M{m}")
        write_line(ser, f"S {m}")
        drain(ser, 1.0)

        print(f">>> DRIVE M{m} at {SPEED} for 2 seconds")
        write_line(ser, f"V {SPEED}")
        drain(ser, 0.5)
        time.sleep(2.0)

        print(f">>> STOP M{m}")
        write_line(ser, "V 0")
        drain(ser, 0.8)

    print("\nDone.")
