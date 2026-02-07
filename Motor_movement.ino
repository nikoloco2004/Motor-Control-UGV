const int PWM1_PIN = 3;
const int DIR1_PIN = 2;
const float PWM_FREQ = 20000.0;

void setup() {
  pinMode(DIR1_PIN, OUTPUT);
  pinMode(PWM1_PIN, OUTPUT);
  analogWriteFrequency(PWM1_PIN, PWM_FREQ);
  analogWrite(PWM1_PIN, 0);
  Serial.begin(115200);
  Serial.println("READY");
}

void loop() {
  if (!Serial.available()) return;

  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) return;

  char letter;
  int value;
  int n = sscanf(line.c_str(), "%c %d", &letter, &value);
  if (!(n == 2 && letter == 'M')) return;

  // Set direction based on sign
  if (value >= 0) digitalWrite(DIR1_PIN, HIGH);
  else digitalWrite(DIR1_PIN, LOW);

  int pwm = abs(value);
  pwm = constrain(pwm, 0, 255);
  analogWrite(PWM1_PIN, pwm);

  Serial.print("DIR=");
  Serial.print(digitalRead(DIR1_PIN));
  Serial.print(" PWM=");
  Serial.println(pwm);
}
