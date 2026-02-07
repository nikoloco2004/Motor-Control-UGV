// 4-Motor Serial Control (Teensy)
// Commands:  M1 <value>  M2 <value>  M3 <value>  M4 <value>
// value: -255..255   (sign = direction, magnitude = PWM)

const float PWM_FREQ = 20000.0;

// --- Pin mapping (CHANGE THESE TO YOUR WIRING) ---
const int PWM_PIN[4] = {3, 4, 5, 6};     // PWM pins for motors 1..4
const int DIR_PIN[4] = {2, 7, 8, 9};     // DIR pins for motors 1..4

void setMotor(int idx, int value) {
  // idx: 0..3
  // value: -255..255

  // Direction by sign
  if (value >= 0) digitalWrite(DIR_PIN[idx], HIGH);
  else           digitalWrite(DIR_PIN[idx], LOW);

  int pwm = abs(value);
  pwm = constrain(pwm, 0, 255);
  analogWrite(PWM_PIN[idx], pwm);

  // Debug print
  Serial.print("M");
  Serial.print(idx + 1);
  Serial.print(" DIR=");
  Serial.print(digitalRead(DIR_PIN[idx]));
  Serial.print(" PWM=");
  Serial.println(pwm);
}

void setup() {
  Serial.begin(115200);

  // Configure pins + PWM frequency
  for (int i = 0; i < 4; i++) {
    pinMode(DIR_PIN[i], OUTPUT);
    pinMode(PWM_PIN[i], OUTPUT);
    analogWriteFrequency(PWM_PIN[i], PWM_FREQ);
    analogWrite(PWM_PIN[i], 0);
    digitalWrite(DIR_PIN[i], LOW);
  }

  Serial.println("READY (4 motors)");
  Serial.println("Commands: M1 <val>, M2 <val>, M3 <val>, M4 <val>  where val=-255..255");
}

void loop() {
  if (!Serial.available()) return;

  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) return;

  // Expect: "M<id> <value>" e.g., "M3 -120"
  char mChar;
  int motorId;
  int value;

  // Parse: M + integer motorId + integer value
  // This sscanf pattern reads 'M' into mChar, then motorId, then value.
  int n = sscanf(line.c_str(), "%c%d %d", &mChar, &motorId, &value);

  if (n != 3 || mChar != 'M') return;
  if (motorId < 1 || motorId > 4) return;

  setMotor(motorId - 1, value);
}
