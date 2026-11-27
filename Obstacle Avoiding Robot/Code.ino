#include <Servo.h>

// ===================== PIN CONFIGURATION =====================
// Ultrasonic Sensor (HC-SR04)
const uint8_t TRIG_PIN  = 9;
const uint8_t ECHO_PIN  = 8;

// Servo Motor
const uint8_t SERVO_PIN = 11;

// L298N Motor Driver (Speed Control Enabled)
const uint8_t ENA_PIN   = 3;  // Right Motor Speed (PWM)
const uint8_t ENB_PIN   = 5;  // Left Motor Speed (PWM)

// Right Motors: OUT1 (-), OUT2 (+)
const uint8_t IN1_PIN   = 10;
const uint8_t IN2_PIN   = 6;
// Left Motors: OUT3 (+), OUT4 (-)
const uint8_t IN3_PIN   = 7;
const uint8_t IN4_PIN   = 12;

// ===================== TUNABLE PARAMETERS =====================
// চাকা উল্টো ঘুরলে তার না খুলে শুধু নিচের মান true করে দিবে
const bool INVERT_RIGHT_MOTOR = false;
const bool INVERT_LEFT_MOTOR  = false;

// Speed Settings (0 থেকে 255 এর মধ্যে)
const uint8_t DRIVE_SPEED     = 160; // সামনে/পেছনে চলার স্পিড
const uint8_t TURN_SPEED      = 175; // ঘোরার (Turn) স্পিড
const uint8_t BRAKE_SPEED     = 255; // হার্ড ব্রেকের স্পিড

// Obstacle & Sensor Settings
const int SAFE_DISTANCE_CM           = 28;
const int MAX_DISTANCE_CM            = 250;
const unsigned long ECHO_TIMEOUT_US  = 25000UL;
const unsigned long PING_INTERVAL_MS = 40;

// Servo Angles
const int SERVO_CENTER_ANGLE   = 90;
const int SERVO_LEFT_ANGLE     = 170;
const int SERVO_RIGHT_ANGLE    = 10;

// Timing Settings
const int SERVO_SHORT_SETTLE_MS = 220;
const int SERVO_LONG_SETTLE_MS  = 380;
const int BRAKE_PULSE_MS        = 45;
const int TURN_90_TIME_MS       = 400;
const int TURN_180_TIME_MS      = 780;
const int REVERSE_TIME_MS       = 350;
const uint8_t MAX_ESCAPE_RETRIES = 2;

// ===================== GLOBAL OBJECTS =====================
Servo scannerServo;
unsigned long lastPingTime = 0;

// ===================== MOTOR CONTROL =====================
void setRightMotor(int direction, uint8_t speed) {
  if (INVERT_RIGHT_MOTOR) {
    direction = -direction;
  }

  if (direction > 0) {
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, HIGH);
    analogWrite(ENA_PIN, speed);
  } else if (direction < 0) {
    digitalWrite(IN1_PIN, HIGH);
    digitalWrite(IN2_PIN, LOW);
    analogWrite(ENA_PIN, speed);
  } else {
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, LOW);
    analogWrite(ENA_PIN, 0);
  }
}

void setLeftMotor(int direction, uint8_t speed) {
  if (INVERT_LEFT_MOTOR) {
    direction = -direction;
  }

  if (direction > 0) {
    digitalWrite(IN3_PIN, HIGH);
    digitalWrite(IN4_PIN, LOW);
    analogWrite(ENB_PIN, speed);
  } else if (direction < 0) {
    digitalWrite(IN3_PIN, LOW);
    digitalWrite(IN4_PIN, HIGH);
    analogWrite(ENB_PIN, speed);
  } else {
    digitalWrite(IN3_PIN, LOW);
    digitalWrite(IN4_PIN, LOW);
    analogWrite(ENB_PIN, 0);
  }
}

void moveForward() {
  setRightMotor(1, DRIVE_SPEED);
  setLeftMotor(1, DRIVE_SPEED);
}

void moveBackward(uint8_t speed = DRIVE_SPEED) {
  setRightMotor(-1, speed);
  setLeftMotor(-1, speed);
}

void stopMotors() {
  setRightMotor(0, 0);
  setLeftMotor(0, 0);
}

void hardBrake() {
  moveBackward(BRAKE_SPEED);
  delay(BRAKE_PULSE_MS);
  stopMotors();
}

void turnLeft(int durationMs) {
  setRightMotor(1, TURN_SPEED);
  setLeftMotor(-1, TURN_SPEED);
  delay(durationMs);
  stopMotors();
}

void turnRight(int durationMs) {
  setRightMotor(-1, TURN_SPEED);
  setLeftMotor(1, TURN_SPEED);
  delay(durationMs);
  stopMotors();
}

void reverseRobot(int durationMs) {
  moveBackward(DRIVE_SPEED);
  delay(durationMs);
  stopMotors();
}

// ===================== ULTRASONIC MEASUREMENT =====================
int readRawDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
  if (duration == 0) {
    return MAX_DISTANCE_CM;
  }

  int distanceCm = (int)(duration / 58UL);
  return (distanceCm > MAX_DISTANCE_CM) ? MAX_DISTANCE_CM : distanceCm;
}

int readFilteredDistanceCm() {
  int r1 = readRawDistanceCm();
  delay(15);
  int r2 = readRawDistanceCm();
  delay(15);
  int r3 = readRawDistanceCm();

  if (r1 > r2) { int t = r1; r1 = r2; r2 = t; }
  if (r2 > r3) { int t = r2; r2 = r3; r3 = t; }
  if (r1 > r2) { int t = r1; r1 = r2; r2 = t; }

  return r2;
}

// ===================== SERVO SCANNING =====================
void scanEnvironment(int &leftDist, int &centerDist, int &rightDist) {
  scannerServo.write(SERVO_CENTER_ANGLE);
  centerDist = readFilteredDistanceCm();

  scannerServo.write(SERVO_LEFT_ANGLE);
  delay(SERVO_SHORT_SETTLE_MS);
  leftDist = readFilteredDistanceCm();

  scannerServo.write(SERVO_RIGHT_ANGLE);
  delay(SERVO_LONG_SETTLE_MS);
  rightDist = readFilteredDistanceCm();

  scannerServo.write(SERVO_CENTER_ANGLE);
  delay(SERVO_SHORT_SETTLE_MS);

  Serial.print(F("Scan -> L: "));
  Serial.print(leftDist);
  Serial.print(F(" | C: "));
  Serial.print(centerDist);
  Serial.print(F(" | R: "));
  Serial.println(rightDist);
}

// ===================== OBSTACLE AVOIDANCE =====================
void avoidObstacle() {
  hardBrake();

  for (uint8_t attempt = 0; attempt <= MAX_ESCAPE_RETRIES; attempt++) {
    int leftDist = 0, centerDist = 0, rightDist = 0;
    scanEnvironment(leftDist, centerDist, rightDist);

    bool leftClear  = (leftDist > SAFE_DISTANCE_CM);
    bool rightClear = (rightDist > SAFE_DISTANCE_CM);

    if (leftClear || rightClear) {
      if (leftDist >= rightDist) {
        Serial.println(F("Action: Turn Left"));
        turnLeft(TURN_90_TIME_MS);
      } else {
        Serial.println(F("Action: Turn Right"));
        turnRight(TURN_90_TIME_MS);
      }
      return;
    }

    if (attempt < MAX_ESCAPE_RETRIES) {
      Serial.println(F("Action: Blocked -> Reversing & Rescanning"));
      reverseRobot(REVERSE_TIME_MS);
    } else {
      Serial.println(F("Action: Dead End -> 180 Turnaround"));
      reverseRobot(REVERSE_TIME_MS);
      turnRight(TURN_180_TIME_MS);
    }
  }
}

// ===================== SETUP & MAIN LOOP =====================
void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(ENA_PIN, OUTPUT);
  pinMode(ENB_PIN, OUTPUT);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(IN3_PIN, OUTPUT);
  pinMode(IN4_PIN, OUTPUT);

  stopMotors();

  scannerServo.attach(SERVO_PIN);
  scannerServo.write(SERVO_CENTER_ANGLE);
  delay(400);

  Serial.println(F("Robot Initialized with Speed Control!"));
}

void loop() {
  unsigned long now = millis();

  if (now - lastPingTime >= PING_INTERVAL_MS) {
    lastPingTime = now;
    int frontDistance = readRawDistanceCm();

    if (frontDistance <= SAFE_DISTANCE_CM) {
      delay(15);
      int confirmDistance = readRawDistanceCm();

      if (confirmDistance <= SAFE_DISTANCE_CM) {
        Serial.print(F("Obstacle at: "));
        Serial.print(confirmDistance);
        Serial.println(F(" cm"));
        avoidObstacle();
      }
    } else {
      moveForward();
    }
  }
}
