import serial, time, threading

PORT = "COM13"
BAUD = 115200

ser = serial.Serial(PORT, BAUD, timeout=0.2)
time.sleep(2)
ser.reset_input_buffer()

def send(cmd: str):
    ser.write((cmd.strip() + "\n").encode())

stop_flag = False

def reader():
    global stop_flag
    last_print = time.time()
    while not stop_flag:
        line = ser.readline().decode(errors="ignore").strip()
        if not line:
            continue

        c_str, deg_str, tgt_str, err_str = line.split(",", 3)

        if time.time() - last_print > 0.1:
            print(
                f"counts={int(c_str):7d}  "
                f"deg={float(deg_str):8.3f}  "
                f"tgt={float(tgt_str):8.3f}  "
                f"err={float(err_str):8.3f}"
            )
            last_print = time.time()

t = threading.Thread(target=reader, daemon=True)
t.start()

try:
    print("Commands: z  |  t,<deg> (ex: t,45). Ctrl+C to quit.")
    while True:
        cmd = input("cmd> ").strip()
        if cmd:
            send(cmd)
except KeyboardInterrupt:
    pass
finally:
    stop_flag = True
    time.sleep(0.1)
    ser.close()
    print("Closed.")