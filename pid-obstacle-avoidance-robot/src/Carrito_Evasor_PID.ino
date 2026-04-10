#include <NewPing.h>
#include <Servo.h>

Servo servoSensor;

#define TRIG_PIN A3
#define ECHO_PIN A2
#define MAX_DISTANCE 200
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

const int ENA = 5;
const int IN1 = 10;
const int IN2 = 8;
const int ENB = 6;
const int IN3 = 11;
const int IN4 = 7;

const double Ts = 0.05;
unsigned long t0;

double ref = 0.30;

const double Ainc = 0.3 * 222.6841;
const double Binc = 0.3 * -44.7737;
const double Cinc = 0.3 * 15.6834;

double ek   = 0.0;
double ekm1 = 0.0;
double ekm2 = 0.0;
double uk   = 0.0;
double ukm1 = 0.0;

double d_ema = 0.0;
const double EMA_ALPHA = 0.2;

const double CTRL_GAIN = 11.5;
const int    PWM_MAX   = 200;
const int    PWM_MIN   = 65;

const double E_DEADBAND_IN  = 0.04;
const double E_DEADBAND_OUT = 0.04;
bool enReposo = false;

bool INVERT_DIR = true;

const int OFFSET_RIGHT_FWD = 21;
const int OFFSET_RIGHT_REV = 20;

static inline float median3(float a, float b, float c) {
  if (a > b) { float t = a; a = b; b = t; }
  if (b > c) { float t = b; b = c; c = t; }
  if (a > b) { float t = a; a = b; b = t; }
  return b;
}

double readDistanceM() {
  unsigned int c1 = sonar.ping_cm(); if (!c1) c1 = MAX_DISTANCE;
  unsigned int c2 = sonar.ping_cm(); if (!c2) c2 = MAX_DISTANCE;
  unsigned int c3 = sonar.ping_cm(); if (!c3) c3 = MAX_DISTANCE;
  float cm = median3(c1, c2, c3);
  return cm / 100.0;
}

void driveSignedPWM(int uSigned) {
  int s = (uSigned >= 0) ? +1 : -1;
  if (INVERT_DIR) s = -s;

  int base = abs(uSigned);
  if (base > PWM_MAX) base = PWM_MAX;

  int valLeft  = base;
  int valRight = base;

  if (s > 0) {
    valRight -= OFFSET_RIGHT_FWD;
    if (valRight < 0) valRight = 0;
  } else {
    valRight -= OFFSET_RIGHT_REV;
    if (valRight < 0) valRight = 0;
  }

  digitalWrite(IN1, (s > 0) ? HIGH : LOW);
  digitalWrite(IN2, (s > 0) ? LOW  : HIGH);
  digitalWrite(IN3, (s > 0) ? HIGH : LOW);
  digitalWrite(IN4, (s > 0) ? LOW  : HIGH);

  analogWrite(ENA, valLeft);
  analogWrite(ENB, valRight);
}

void setup() {
  Serial.begin(115200);

  servoSensor.attach(9);
  servoSensor.write(90);
  delay(200);

  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  digitalWrite(IN1, LOW);  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, LOW);

  d_ema = readDistanceM();
  t0 = millis();
}

void loop() {
  if (millis() - t0 < (unsigned long)(Ts * 1000.0)) return;
  t0 += (unsigned long)(Ts * 1000.0);

  double dist = readDistanceM();
  if (dist > 2.0) dist = 2.0;

  d_ema = (1.0 - EMA_ALPHA) * d_ema + EMA_ALPHA * dist;

  ek = ref - d_ema;

  if (enReposo) {
    if (fabs(ek) <= E_DEADBAND_OUT) {
      driveSignedPWM(0);

      uk   = 0.0;
      ukm1 = 0.0;
      ekm1 = 0.0;
      ekm2 = 0.0;

      Serial.print(ref, 3);
      Serial.print(" ");
      Serial.println(d_ema, 3);
      return;
    } else {
      enReposo = false;
    }
  } else {
    if (fabs(ek) <= E_DEADBAND_IN) {
      enReposo = true;

      driveSignedPWM(0);

      uk   = 0.0;
      ukm1 = 0.0;
      ekm1 = 0.0;
      ekm2 = 0.0;

      Serial.print(ref, 3);
      Serial.print(" ");
      Serial.println(d_ema, 3);
      return;
    }
  }

  uk = ukm1 + Ainc * ek + Binc * ekm1 + Cinc * ekm2;

  if (uk >  10.0) uk =  10.0;
  if (uk < -10.0) uk = -10.0;

  double uAbs = fabs(uk) * CTRL_GAIN;
  int pwm = (int)uAbs;
  if (pwm > PWM_MAX) pwm = PWM_MAX;
  if (pwm < PWM_MIN) pwm = PWM_MIN;

  int sign = (uk >= 0.0) ? +1 : -1;
  int uSigned = sign * pwm;

  driveSignedPWM(uSigned);

  ekm2 = ekm1;
  ekm1 = ek;
  ukm1 = uk;

  Serial.print(ref, 3);
  Serial.print(" ");
  Serial.println(d_ema, 3);
}
