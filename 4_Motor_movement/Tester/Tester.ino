const float PWM_FREQ = 20000.0;

// Single physical wheel output pins (CHANGE THESE)
const int PWM_OUT_PIN = 3;
const int DIR_OUT_PIN = 2;

int activeMotor = 1;  // which command ID (M1..M4) controls the single wheel

void setup() {
  pinMode(DIR_OUT_PIN, OUTPUT);
  pinMode(PWM_OUT_PIN, OUTPUT);
  analogWriteFrequency(PWM_OUT_PIN, PWM_FREQ);
  analogWrite(PWM_OUT_PIN, 0);
  digitalWrite(DIR_OUT_PIN, LOW);

  Serial.begin(115200);
  Serial.println("READY (single wheel mapping)");
  Serial.println("Use: S <1..4> to select motor ID. Then: M1..M4 <val> (-255..255).");
}

void driveWheel(int value) {
  if (value >= 0) digitalWrite(DIR_OUT_PIN, HIGH);
  else           digitalWrite(DIR_OUT_PIN, LOW);

  int pwm = abs(value);
  pwm = constrain(pwm, 0, 255);
  analogWrite(PWM_OUT_PIN, pwm);

  Serial.print("ACTIVE=M");
  Serial.print(activeMotor);
  Serial.print(" DIR=");
  Serial.print(digitalRead(DIR_OUT_PIN));
  Serial.print(" PWM=");
  Serial.println(pwm);
}

void stopWheel() {
  analogWrite(PWM_OUT_PIN, 0);
}

void loop() {
  if (!Serial.available()) return;

  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) return;

  // Command formats:
  // S <id>        (select active motor 1..4)
  // M<id> <value> (set motor id to value)
  char c;
  int id;
  int value;

  // Try select command: "S 3"
  if (sscanf(line.c_str(), "%c %d", &c, &id) == 2 && c == 'S') {
    if (id >= 1 && id <= 4) {
      activeMotor = id;
      Serial.print("SELECTED M");
      Serial.println(activeMotor);
      stopWheel();
    }
    return;
  }

  // Try motor command: "M2 -150"
  if (sscanf(line.c_str(), "%c%d %d", &c, &id, &value) == 3 && c == 'M') {
    // Only drive the wheel if command matches selected motor
    if (id == activeMotor) driveWheel(value);
    else stopWheel();  // force off if it's not the active one
    return;
  }
}
