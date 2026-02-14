# Motor-Control-UGV (Beginner Guide)

Control **DC motors for a robot (UGV)** using:

- **PC or Jetson Nano (Python)** → sends commands over **USB serial**
- **Teensy 4.1** → interprets commands and outputs **PWM + DIR**
- **Cytron MDD10A (REV2.0)** → drives motors using **12V power**

This repo is written for **absolute beginners** and explains *why* things work, not just *what to plug in*.

---

## 1) What you’re building (big picture)

Think of the system as layers:

1. **Python (PC or Jetson Nano)**  
   Decides how the robot should move:
   - forward
   - backward
   - strafe
   - rotate
   - diagonal

2. Python sends **simple text commands** over USB, for example:
M1 180
M2 -180
M3 180
M4 -180


3. **Teensy 4.1**:
- reads those commands
- decides **direction** (DIR pin)
- decides **speed** (PWM pin)

4. **Cytron MDD10A motor driver**:
- takes **12V power**
- uses Teensy’s PWM + DIR signals
- pushes real current into the motors

5. **Motors move**

Important idea:
- ❌ PC / Jetson / Teensy **do NOT power motors**
- ✅ The **12V power supply** powers motors through the MDD10A

---

## 2) Key terms (plain English)

- **Serial / UART**  
A way to send text messages between devices.  
Over USB, this looks like a COM port (Windows) or `/dev/ttyACM0` (Linux).

- **PWM (Pulse Width Modulation)**  
A fast ON/OFF signal.
- 0   = stopped  
- 255 = full speed  

- **DIR (Direction)**  
HIGH or LOW → forward or backward.

- **Common Ground**  
Teensy GND and MDD10A GND must be connected so signals make sense.

---

## 3) Hardware used

- Teensy 4.1
- Cytron MDD10A REV2.0
- Pololu 12V DC gearmotor (with encoder – encoder ignored for now)
- 12V bench power supply

---

## 4) Safety rules (read this)

- First tests: **wheel off the ground**
- Power-up order:
1) Plug **USB (PC/Jetson → Teensy)**
2) Verify wiring
3) Turn on **12V power supply**
- Power-down order:
1) Turn off **12V**
2) Unplug USB if needed

---

## 5) Wiring (single motor, Channel 1)

### A) Motor → MDD10A

Use **ONLY the motor power wires**:

- Red  → M1A  
- Black → M1B  

Swapping them reverses direction (safe).

---

### B) Power supply → MDD10A

Set bench supply:
- Voltage: **12.0V**
- Current limit: **6–8A**

Connect:
- Supply + → POWER+
- Supply – → POWER–

---

### C) Teensy → MDD10A (control)

MDD10A control header:
GND | PWM2 | DIR2 | PWM1 | DIR1


Using **Motor 1**:

- Teensy GND → MDD10A GND  ✅ REQUIRED
- Teensy pin 3 → PWM1
- Teensy pin 2 → DIR1

---

## 6) Teensy firmware (single-wheel mapping)

This firmware supports **bench testing** with ONE physical wheel.

Key idea:
- Python sends commands for **M1, M2, M3, M4**
- Teensy lets you **select which M# controls the single wheel**
- This lets you verify wiring + direction **one motor at a time**

### Supported serial commands

- Select motor ID:
S 1 # select M1
S 2 # select M2


- Drive motor:
M1 180
M2 -150


### Teensy code

```cpp
# define PWM1_PIN 3
# define DIR1_PIN 2
# define PWM_FREQ 20000.0

int activeMotor = 1;

void setup() {
pinMode(DIR1_PIN, OUTPUT);
pinMode(PWM1_PIN, OUTPUT);

analogWriteFrequency(PWM1_PIN, PWM_FREQ);
analogWrite(PWM1_PIN, 0);

Serial.begin(115200);
Serial.println("READY (single wheel mapping)");
}

void driveWheel(int value) {
if (value >= 0) digitalWrite(DIR1_PIN, HIGH);
else digitalWrite(DIR1_PIN, LOW);

int pwm = abs(value);
pwm = constrain(pwm, 0, 255);
analogWrite(PWM1_PIN, pwm);

Serial.print("ACTIVE=M");
Serial.print(activeMotor);
Serial.print(" PWM=");
Serial.println(pwm);
}

void loop() {
if (!Serial.available()) return;

String line = Serial.readStringUntil('\n');
line.trim();

char c;
int id, value;

if (sscanf(line.c_str(), "%c %d", &c, &id) == 2 && c == 'S') {
  if (id >= 1 && id <= 4) activeMotor = id;
  return;
}

if (sscanf(line.c_str(), "%c%d %d", &c, &id, &value) == 3 && c == 'M') {
  if (id == activeMotor) driveWheel(value);
  else analogWrite(PWM1_PIN, 0);
}
}
7) Python control (PC or Jetson Nano)
Serial ports
Windows: COM13

Jetson Nano: /dev/ttyACM0

Why inversion exists
Because motors are mirrored left vs right, the same signal does NOT always mean the same physical direction.

To fix this, Python applies per-motor inversion:

self.inv = [1, -1, 1, -1]  # FL, FR, BL, BR
Now:

forward() always means forward

backward() always means backward

no guessing signs in movement functions

8) Bench testing (very important)
bench_test_moves.py:

prints what each motor should do

selects M1 → M4 one at a time

spins only the selected motor

This is how you:

confirm wiring

confirm direction

confirm inversion

Run:

python bench_test_moves.py
9) Real robot mode (later)
Once all motors are connected:

remove single-wheel mapping firmware

use full 4-motor Teensy firmware

Python logic does not change

That’s the goal of this architecture.

10) PC vs Jetson Nano
Nothing changes logically.

Only differences:

serial port name

Linux permissions (dialout group)

deployment (Jetson runs code on boot)

The protocol and code stay the same.

Final mindset
Teensy = real-time motor control

Python = brain / planner

MDD10A = muscle

Bench testing prevents magic smoke

If you understand this README, you understand 90% of robotics motor control.


---

If you want, next we can:
- add a **diagram**
- split README into **Beginner / Advanced**
- or add a **“What breaks most often”** troubleshooting section
