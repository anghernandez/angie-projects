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

const double Ainc = 0.25 * 222.6841;
const double Binc = 0.25 * -44.7737;
const double Cinc = 0.25 * 15.6834;

double ek   = 0.0;
double ekm1 = 0.0;
double ekm2 = 0.0;
double uk   = 0.0;
double ukm1 = 0.0;

double d_ema = 0.0;
const double EMA_ALPHA = 0.2;

const double CTRL_GAIN = 13.0;
const int    PWM_MAX   = 220;
const int    PWM_MIN   = 70;

const double REF_BAND = 0.02;

const double D_EMERGENCY = 0.15;

bool INVERT_DIR = true;

const int OFFSET_RIGHT_FWD = 21;
const int OFFSET_RIGHT_REV = 20;

const int SERVO_CENTER = 90;
const int SERVO_LEFT1  = 120;
const int SERVO_LEFT2  = 140;
const int SERVO_RIGHT1 = 60;
const int SERVO_RIGHT2 = 40;

const int TURN_PWM     = 130;
const int TURN_STEPS   = 12;

enum Mode { MODE_FOLLOW, MODE_AVOID_TURN };
Mode mode = MODE_FOLLOW;
int  turnDir   = 0;
int  turnSteps = 0;

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

double meanDistanceAtAngle(int angleDeg, int nSamples = 3) {
  servoSensor.write(angleDeg);
  delay(150);
  double sum = 0.0;
  for (int i = 0; i < nSamples; i++) {
    double d = readDistanceM();
    if (d > 2.0) d = 2.0;
    sum += d;
    delay(40);
  }
  return sum / (double)nSamples;
}

double scanSideMean(int side) {
  double m1, m2;
  if (side < 0) {
    m1 = meanDistanceAtAngle(SERVO_LEFT1);
    m2 = meanDistanceAtAngle(SERVO_LEFT2);
  } else {
    m1 = meanDistanceAtAngle(SERVO_RIGHT1);
    m2 = meanDistanceAtAngle(SERVO_RIGHT2);
  }
  return 0.5 * (m1 + m2);
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

void driveTurnInPlace(int dir, int pwm) {
  if (pwm > PWM_MAX) pwm = PWM_MAX;
  if (pwm < PWM_MIN) pwm = PWM_MIN;

  if (dir > 0) {
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
  } else {
    digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  }

  analogWrite(ENA, pwm);
  analogWrite(ENB, pwm);
}

void resetPID() {
  uk   = 0.0;
  ukm1 = 0.0;
  ekm1 = 0.0;
  ekm2 = 0.0;
}

void startAvoidance() {
  driveSignedPWM(0);
  resetPID();

  double dLeftMean  = scanSideMean(-1);
  double dRightMean = scanSideMean(+1);

  servoSensor.write(SERVO_CENTER);
  delay(150);

  if (dLeftMean > dRightMean) turnDir = -1;
  else                        turnDir = +1;

  turnSteps = TURN_STEPS;
  mode = MODE_AVOID_TURN;

  t0 = millis();
}

void setup() {
  Serial.begin(115200);

  servoSensor.attach(9);
  servoSensor.write(SERVO_CENTER);
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

  if (mode == MODE_FOLLOW) {
    ek = ref - d_ema;

    if (d_ema < D_EMERGENCY) {
      startAvoidance();
      Serial.print(ref, 3);
      Serial.print(" ");
      Serial.println(d_ema, 3);
      return;
    }

    if (fabs(ek) <= REF_BAND) {
      startAvoidance();
      Serial.print(ref, 3);
      Serial.print(" ");
      Serial.println(d_ema, 3);
      return;
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
  else if (mode == MODE_AVOID_TURN) {
    if (turnSteps > 0) {
      driveTurnInPlace(turnDir, TURN_PWM);
      turnSteps--;
    } else {
      driveSignedPWM(0);
      resetPID();
      mode = MODE_FOLLOW;
      servoSensor.write(SERVO_CENTER);
      delay(100);
    }

    Serial.print(ref, 3);
    Serial.print(" ");
    Serial.println(d_ema, 3);
  }
}
