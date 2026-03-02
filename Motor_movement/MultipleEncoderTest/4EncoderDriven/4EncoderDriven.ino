#include <Encoder.h>
#include <math.h>   // fabs()

// =====================================================
// Teensy 4.1 + 4x MD10A + 4x Quadrature Encoders
// Control: P-only position to absolute target degrees
//
// Pin plan (recommended):
//   Enc0 A=2  B=3      Motor0 PWM=10 DIR=22
//   Enc1 A=4  B=5      Motor1 PWM=11 DIR=23
//   Enc2 A=6  B=7      Motor2 PWM=12 DIR=24
//   Enc3 A=8  B=9      Motor3 PWM=13 DIR=25
//
// Serial commands:
//   z         -> zero ALL + target=0 + enabled + stop
//   z,<i>     -> zero wheel i (0..3)
//   s         -> stop/disable ALL
//   s,<i>     -> stop/disable wheel i
//   x         -> full reset ALL (zero + target=0 + disabled)
//   x,<i>     -> full reset wheel i
//   t,<deg>   -> set target for ALL wheels (enables them)
//   ta,<deg>  -> set target for ALL wheels (same as t,<deg>)
//   t<i>,<deg>-> set target for wheel i only (t0..t3) (enables it)
//
// Telemetry @ 100 Hz (4 lines each cycle):
//   i,counts,deg,target,error
// =====================================================

constexpr int N = 4;

// ---------- Pins ----------
constexpr int ENC_A[N]   = {2, 4, 6, 8};
constexpr int ENC_B[N]   = {3, 5, 7, 9};

constexpr int PWM_PIN[N] = {10, 11, 12, 13};   // PWM-capable on Teensy 4.1
constexpr int DIR_PIN[N] = {22, 23, 24, 25};   // digital

// If any wheel spins the wrong way, flip its entry to true
constexpr bool DIR_INVERT[N] = {false, false, false, false};

// ---------- Encoder scaling ----------
constexpr float GEAR_RATIO   = 70.0f;
constexpr float CPR_MOTOR    = 64.0f;   // your stated CPR (see note in chat)
constexpr float DEG_PER_COUNT = 360.0f / (CPR_MOTOR * GEAR_RATIO);

// ---------- Control ----------
constexpr float Kp          = 2.0f;
constexpr int   MAX_PWM     = 180;      // 0..255
constexpr float DEADBAND_DEG = 0.5f;
constexpr float MAX_TARGET_DEG = 5000.0f;

// ---------- Encoders ----------
Encoder enc0(ENC_A[0], ENC_B[0]);
Encoder enc1(ENC_A[1], ENC_B[1]);
Encoder enc2(ENC_A[2], ENC_B[2]);
Encoder enc3(ENC_A[3], ENC_B[3]);
Encoder* enc[N] = { &enc0, &enc1, &enc2, &enc3 };

// ---------- State ----------
long  zeroOffset[N]   = {0, 0, 0, 0};
float targetDeg[N]    = {0, 0, 0, 0};
bool  motorEnabled[N] = {true, true, true, true};

elapsedMillis telemTimer;

// ---------------- Motor helpers ----------------
void motorStop(int i) {
  analogWrite(PWM_PIN[i], 0);
}

void motorDrive(int i, float errDeg) {
  if (!motorEnabled[i]) { motorStop(i); return; }

  if (fabs(errDeg) < DEADBAND_DEG) {
    motorStop(i);
    return;
  }

  float u = Kp * errDeg;           // signed command
  int pwm = (int)fabs(u);
  if (pwm > MAX_PWM) pwm = MAX_PWM;

  bool dir = (u >= 0) ? LOW : HIGH;    // same convention as your 1-motor code
  if (DIR_INVERT[i]) dir = !dir;

  digitalWrite(DIR_PIN[i], dir);
  analogWrite(PWM_PIN[i], pwm);
}

void zeroWheel(int i) {
  zeroOffset[i] = enc[i]->read();
  targetDeg[i] = 0.0f;
  motorEnabled[i] = true;
  motorStop(i);
}

void stopWheel(int i) {
  motorEnabled[i] = false;
  motorStop(i);
}

void resetWheel(int i) {
  motorEnabled[i] = false;
  motorStop(i);
  zeroOffset[i] = enc[i]->read();
  targetDeg[i] = 0.0f;
}

void setTargetWheel(int i, float deg) {
  if (deg >  MAX_TARGET_DEG) deg =  MAX_TARGET_DEG;
  if (deg < -MAX_TARGET_DEG) deg = -MAX_TARGET_DEG;

  targetDeg[i] = deg;
  motorEnabled[i] = true;
}

// ---------------- Parsing helpers ----------------
bool parseOptionalIndex(const String& cmd, int& idxOut) {
  int comma = cmd.indexOf(',');
  if (comma < 0) return false;

  String num = cmd.substring(comma + 1);
  idxOut = num.toInt();
  return (idxOut >= 0 && idxOut < N);
}

// Accepts: t,<deg>  ta,<deg>  t0,<deg> ... t3,<deg>
bool parseTargetCommand(const String& cmd, int& which, float& deg) {
  if (cmd.length() < 3) return false;
  if (!(cmd[0] == 't' || cmd[0] == 'T')) return false;

  int comma = cmd.indexOf(',');
  if (comma < 0) return false;

  String head = cmd.substring(0, comma);   // "t", "ta", "t2"
  String val  = cmd.substring(comma + 1);  // "<deg>"

  deg = val.toFloat();

  if (head.length() == 1) {               // "t,<deg>" => all
    which = -1;
    return true;
  }
  if (head.length() == 2 && (head[1] == 'a' || head[1] == 'A')) { // "ta,<deg>" => all
    which = -1;
    return true;
  }
  if (head.length() == 2 && isDigit(head[1])) { // "t2,<deg>" => wheel 2
    which = head[1] - '0';
    return (which >= 0 && which < N);
  }

  return false;
}

// =====================================================
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {}
  Serial.setTimeout(10);

  for (int i = 0; i < N; i++) {
    pinMode(DIR_PIN[i], OUTPUT);
    pinMode(PWM_PIN[i], OUTPUT);
    motorStop(i);
    zeroOffset[i] = enc[i]->read();
  }
}

void loop() {
  // ----- Commands -----
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    // z / z,i
    if (cmd.equalsIgnoreCase("z")) {
      for (int i = 0; i < N; i++) zeroWheel(i);
    } else if (cmd.startsWith("z") || cmd.startsWith("Z")) {
      int i;
      if (parseOptionalIndex(cmd, i)) zeroWheel(i);
    }

    // s / s,i
    else if (cmd.equalsIgnoreCase("s")) {
      for (int i = 0; i < N; i++) stopWheel(i);
    } else if (cmd.startsWith("s") || cmd.startsWith("S")) {
      int i;
      if (parseOptionalIndex(cmd, i)) stopWheel(i);
    }

    // x / x,i
    else if (cmd.equalsIgnoreCase("x")) {
      for (int i = 0; i < N; i++) resetWheel(i);
    } else if (cmd.startsWith("x") || cmd.startsWith("X")) {
      int i;
      if (parseOptionalIndex(cmd, i)) resetWheel(i);
    }

    // t...
    else {
      int which;
      float deg;
      if (parseTargetCommand(cmd, which, deg)) {
        if (which == -1) {
          for (int i = 0; i < N; i++) setTargetWheel(i, deg);
        } else {
          setTargetWheel(which, deg);
        }
      }
    }
  }

  // ----- Control + Telemetry @ 100 Hz -----
  if (telemTimer >= 10) {
    telemTimer = 0;

    for (int i = 0; i < N; i++) {
      long  counts = enc[i]->read() - zeroOffset[i];
      float deg    = counts * DEG_PER_COUNT;
      float err    = targetDeg[i] - deg;

      motorDrive(i, err);

      // CSV: i,counts,deg,target,error
      Serial.print(i);
      Serial.print(",");
      Serial.print(counts);
      Serial.print(",");
      Serial.print(deg, 3);
      Serial.print(",");
      Serial.print(targetDeg[i], 3);
      Serial.print(",");
      Serial.println(err, 3);
    }
  }
}