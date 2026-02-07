# Motor-Control-UGV





\# Motor-Control-UGV (Beginner Guide)

Control a \*\*12V DC motor\*\* using:

\- \*\*PC (Python)\*\* → sends commands over \*\*USB serial\*\* (UART-style)

\- \*\*Teensy 4.1\*\* → reads commands and outputs \*\*PWM + DIR\*\*

\- \*\*Cytron MDD10A (REV2.0)\*\* → uses \*\*12V power\*\* to drive the motor



This repo is meant to be \*\*complete-beginner friendly\*\*.



---



\## 1) What you’re building (big picture)

Think of it like a chain:



1\. \*\*Your PC\*\* decides what you want: forward / stop / backward  

2\. PC sends a small text message over USB like:  

&nbsp;  - `M 150`  (forward)

&nbsp;  - `M 0`    (stop)

&nbsp;  - `M -150` (backward)

3\. \*\*Teensy\*\* reads that message and converts it into two electrical signals:

&nbsp;  - \*\*DIR\*\* (direction): forward vs backward

&nbsp;  - \*\*PWM\*\* (speed): how fast

4\. \*\*MDD10A motor driver\*\* is the “muscle”:

&nbsp;  - It takes the \*\*12V power supply\*\*

&nbsp;  - It uses PWM + DIR from the Teensy to push real power into the motor

5\. The \*\*motor moves\*\*.



Key idea:

\- \*\*Teensy and PC do NOT power the motor.\*\*

\- The \*\*12V power supply powers the motor\*\* through the MDD10A.



---



\## 2) Terms (simple definitions)

\- \*\*UART / Serial\*\*: A way devices send text/data back and forth. On a PC, this usually happens over USB as a “COM port”.

\- \*\*PWM (Pulse Width Modulation)\*\*: A fast ON/OFF signal. More ON-time = faster motor.

\- \*\*DIR (Direction)\*\*: A simple HIGH/LOW signal that tells the motor which way to spin.

\- \*\*Common Ground (shared GND)\*\*: The Teensy and motor driver must share the same “0V reference” so signals make sense.



---



\## 3) Hardware used

\- Teensy 4.1

\- Cytron MDD10A REV2.0 motor driver

\- Pololu 12V DC gearmotor with encoder  

&nbsp; (Motor wires: \*\*Red/Black\*\*. Encoder wires: ignore for now.)

\- DC bench power supply set to \*\*12V\*\*



---



\## 4) Safety first (do this)

\- First test: keep the motor \*\*unloaded\*\* (wheel off ground or motor free).

\- Always power in this order:

&nbsp; 1) Plug in \*\*USB\*\* (PC → Teensy)

&nbsp; 2) Double-check wiring

&nbsp; 3) Turn on \*\*12V power supply\*\* last

\- To shut down:

&nbsp; 1) Turn off \*\*12V power supply\*\*

&nbsp; 2) Unplug USB if needed



---



\## 5) Wiring (ONE motor on Channel 1)

\### A) Motor → MDD10A (motor output terminals)

Your Pololu motor has multiple wires. For spinning the motor you ONLY use:

\- \*\*Red = motor power\*\*

\- \*\*Black = motor power\*\*



Connect:

\- \*\*Red → M1A\*\*

\- \*\*Black → M1B\*\*



> If “forward” ends up reversed, swapping Red/Black reverses direction (safe).



\### B) Power supply → MDD10A (power input terminals)

Set your DC supply:

\- \*\*Voltage:\*\* 12.0V

\- \*\*Current limit:\*\* 6–8A (motor can draw several amps at startup)



Connect:

\- DC supply \*\*+ (positive)\*\* → \*\*POWER+\*\*

\- DC supply \*\*– (negative / GND)\*\* → \*\*POWER-\*\*



\### C) Teensy → MDD10A (control header)

The MDD10A control header is labeled:

`GND | PWM2 | DIR2 | PWM1 | DIR1`



We are using \*\*Motor 1\*\*, so use \*\*PWM1\*\* and \*\*DIR1\*\*.



Connect:

\- \*\*Teensy GND → MDD10A GND\*\*  ✅ (required)

\- \*\*Teensy pin 3 → MDD10A PWM1\*\*

\- \*\*Teensy pin 2 → MDD10A DIR1\*\*



> If Teensy is NOT grounded to the MDD10A, direction may not work correctly.



\### D) Encoder wires (ignore for now)

Pololu encoder wires (do not connect yet):

\- Green / Blue / Yellow / White = encoder signals/power  

Leave them disconnected for basic movement tests.



---



\## 6) Teensy firmware (upload to Teensy 4.1)

This program makes the Teensy:

\- read serial commands like `M -150`

\- set DIR based on sign

\- set PWM based on magnitude

\- print debug output back like `DIR=0 PWM=150`



Create a sketch in Arduino IDE (with Teensyduino) and upload:



```cpp

const int PWM1\_PIN = 3;     // Teensy pin 3 -> MDD10A PWM1

const int DIR1\_PIN = 2;     // Teensy pin 2 -> MDD10A DIR1

const float PWM\_FREQ = 20000.0; // 20 kHz PWM



void setup() {

&nbsp; pinMode(DIR1\_PIN, OUTPUT);

&nbsp; pinMode(PWM1\_PIN, OUTPUT);



&nbsp; analogWriteFrequency(PWM1\_PIN, PWM\_FREQ);

&nbsp; analogWrite(PWM1\_PIN, 0);



&nbsp; Serial.begin(115200);

&nbsp; Serial.println("READY");

}



void loop() {

&nbsp; if (!Serial.available()) return;



&nbsp; String line = Serial.readStringUntil('\\n');

&nbsp; line.trim();

&nbsp; if (line.length() == 0) return;



&nbsp; char letter;

&nbsp; int value;

&nbsp; int n = sscanf(line.c\_str(), "%c %d", \&letter, \&value);

&nbsp; if (!(n == 2 \&\& letter == 'M')) return;



&nbsp; // Direction from sign

&nbsp; if (value >= 0) digitalWrite(DIR1\_PIN, HIGH);

&nbsp; else digitalWrite(DIR1\_PIN, LOW);



&nbsp; // Speed from magnitude

&nbsp; int pwm = abs(value);

&nbsp; pwm = constrain(pwm, 0, 255);

&nbsp; analogWrite(PWM1\_PIN, pwm);



&nbsp; // Debug print back to PC

&nbsp; Serial.print("DIR=");

&nbsp; Serial.print(digitalRead(DIR1\_PIN));

&nbsp; Serial.print(" PWM=");

&nbsp; Serial.println(pwm);

}



