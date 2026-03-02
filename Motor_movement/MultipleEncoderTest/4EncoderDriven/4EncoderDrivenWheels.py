import serial, time, threading

PORT = "COM13"
BAUD = 115200
N_WHEELS = 4

ser = serial.Serial(PORT, BAUD, timeout=0.2)
time.sleep(2)
ser.reset_input_buffer()

def send(cmd: str):
    ser.write((cmd.strip() + "\n").encode())

stop_flag = False

# Store latest telemetry per wheel: (counts, deg, tgt, err, timestamp)
latest = {i: None for i in range(N_WHEELS)}
lock = threading.Lock()

def reader():
    global stop_flag
    last_print = time.time()

    while not stop_flag:
        line = ser.readline().decode(errors="ignore").strip()
        if not line:
            continue

        parts = line.split(",")
        # Expect: i,counts,deg,target,error
        if len(parts) != 5:
            continue

        try:
            i = int(parts[0])
            if i < 0 or i >= N_WHEELS:
                continue

            counts = int(parts[1])
            deg    = float(parts[2])
            tgt    = float(parts[3])
            err    = float(parts[4])
        except ValueError:
            continue

        with lock:
            latest[i] = (counts, deg, tgt, err, time.time())

        # Print ~10 Hz
        if time.time() - last_print > 0.1:
            with lock:
                rows = []
                for k in range(N_WHEELS):
                    v = latest[k]
                    if v is None:
                        rows.append(f"[{k}] (no data)")
                    else:
                        c, d, t, e, ts = v
                        rows.append(
                            f"[{k}] c={c:7d} d={d:8.3f} t={t:8.3f} e={e:8.3f}"
                        )
            print(" | ".join(rows))
            last_print = time.time()

t = threading.Thread(target=reader, daemon=True)
t.start()

try:
    print("Commands:")
    print("  z        (zero all)      | z,<i>     (zero one)")
    print("  s        (stop all)      | s,<i>     (stop one)")
    print("  x        (reset all)     | x,<i>     (reset one)")
    print("  t,<deg>  (target all)    | t<i>,<deg> (target one, i=0..3)")
    print("Ctrl+C to quit.\n")

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