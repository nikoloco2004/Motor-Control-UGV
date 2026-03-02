#include <Encoder.h>
#include <math.h>   // for fabs()

constexpr int ENC_A = 2;
constexpr int ENC_B = 3;

// MD10A pins
constexpr int PWM_PIN = 5;   // PWM-capable
constexpr int DIR_PIN = 6;

// Encoder scaling
constexpr float GEAR_RATIO = 70.0f;
constexpr float CPR_MOTOR  = 64.0f;  // x4 decode
constexpr float DEG_PER_COUNT = 360.0f / (CPR_MOTOR * GEAR_RATIO);

// Control (tune)
constexpr float Kp = 2.0f;
constexpr int   MAX_PWM = 180;        // 0..255
constexpr float DEADBAND_DEG = 0.5f;

// Safety clamp for large targets (allows >360 naturally)
constexpr float MAX_TARGET_DEG = 5000.0f;

Encoder enc(ENC_A, ENC_B);

long  zeroOffset = 0;
float targetDeg  = 0.0f;
bool  motorEnabled = true;

elapsedMillis t;

void motorStop() {
  analogWrite(PWM_PIN, 0);
}

void motorDrive(float errDeg) {
  if (!motorEnabled) { motorStop(); return; }

  if (fabs(errDeg) < DEADBAND_DEG) {
    motorStop();
    return;
  }

  float u = Kp * errDeg;          // signed command
  int pwm = (int)fabs(u);
  if (pwm > MAX_PWM) pwm = MAX_PWM;

  // Direction (flipped so positive error moves toward target)
  digitalWrite(DIR_PIN, (u >= 0) ? LOW : HIGH);
  analogWrite(PWM_PIN, pwm);
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {}
  Serial.setTimeout(10);

  pinMode(DIR_PIN, OUTPUT);
  pinMode(PWM_PIN, OUTPUT);
  motorStop();
}

void loop() {
  // Commands:
  // z        -> zero position + target=0 + stop motor (enabled)
  // s        -> stop motor output (disable motor until new target)
  // x        -> stop + zero position + target=0 (full reset, stays disabled)
  // t,<deg>  -> set ABSOLUTE target degrees (can be >360), enables motor
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.equalsIgnoreCase("z")) {
      zeroOffset = enc.read();
      targetDeg = 0.0f;
      motorEnabled = true;
      motorStop();
    }
    else if (cmd.equalsIgnoreCase("s")) {
      motorEnabled = false;
      motorStop();
    }
    else if (cmd.equalsIgnoreCase("x")) {
      motorEnabled = false;
      motorStop();
      zeroOffset = enc.read();
      targetDeg = 0.0f;
      // keep disabled after x so it stays safely stopped
    }
    else if (cmd.length() > 2 && (cmd[0] == 't' || cmd[0] == 'T') && cmd[1] == ',') {
      float cmdTarget = cmd.substring(2).toFloat();

      // Safety clamp (still allows multi-turn targets like 720, 1440, etc.)
      if (cmdTarget >  MAX_TARGET_DEG) cmdTarget =  MAX_TARGET_DEG;
      if (cmdTarget < -MAX_TARGET_DEG) cmdTarget = -MAX_TARGET_DEG;

      targetDeg = cmdTarget;
      motorEnabled = true;  // new target re-enables motor
    }
  }

  // Control + Telemetry @ 100 Hz
  if (t >= 10) {
    t = 0;

    long  counts = enc.read() - zeroOffset;
    float deg    = counts * DEG_PER_COUNT;
    float err    = targetDeg - deg;

    motorDrive(err);

    // CSV-only: counts,deg,target,error
    Serial.print(counts);
    Serial.print(",");
    Serial.print(deg, 3);
    Serial.print(",");
    Serial.print(targetDeg, 3);
    Serial.print(",");
    Serial.println(err, 3);
  }
}