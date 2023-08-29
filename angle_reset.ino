#include <Arduino.h>

const int CW_PIN = 5;
const int CCW_PIN = 6;
const int RIGHT_SENSOR_PIN = 11;
const int LEFT_SENSOR_PIN = 10;

const float STEP_ANGLE = 0.72;
const int SENSOR_DELAY = 1000;
const int STEP_DELAY = 10000;

void setup() {
  pinMode(CW_PIN, OUTPUT);
  pinMode(CCW_PIN, OUTPUT);
  pinMode(RIGHT_SENSOR_PIN, INPUT);
  pinMode(LEFT_SENSOR_PIN, INPUT);
  Serial.begin(115200);
}

void rotateMotor(int steps, bool clockwise) {
  int sensorPin = clockwise ? RIGHT_SENSOR_PIN : LEFT_SENSOR_PIN;
  int interruptSteps = steps;

  digitalWrite(clockwise ? CCW_PIN : CW_PIN, HIGH);

  for (int i = 0; i < steps; i++) {
    if (digitalRead(sensorPin) == LOW) {
      interruptSteps = i;
      break;
    }

    digitalWrite(clockwise ? CCW_PIN : CW_PIN, HIGH);
    delayMicroseconds(STEP_DELAY);
    digitalWrite(clockwise ? CCW_PIN : CW_PIN, LOW);
    delayMicroseconds(STEP_DELAY);
  }

  digitalWrite(CW_PIN, LOW);
  digitalWrite(CCW_PIN, LOW);

  if (interruptSteps > 0) {
    int interruptedAngle = interruptSteps * STEP_ANGLE;
    Serial.println("Interrupted: " + String(clockwise ? "CW" : "CCW") + ", " + String(interruptedAngle));
    delay(SENSOR_DELAY);
    digitalWrite(clockwise ? CW_PIN : CCW_PIN, HIGH);
    delayMicroseconds(STEP_DELAY * 8);
    digitalWrite(clockwise ? CW_PIN : CCW_PIN, LOW);
    delay(SENSOR_DELAY);
    Serial.println("Restarted: " + String(clockwise ? "CCW" : "CW") + ", 8");
  }
}

void resetMotor() {
  while (digitalRead(RIGHT_SENSOR_PIN) == HIGH) {
    digitalWrite(CCW_PIN, HIGH);
    digitalWrite(CW_PIN, LOW);
    delayMicroseconds(STEP_DELAY);
    digitalWrite(CCW_PIN, LOW);
    digitalWrite(CW_PIN, LOW);
    delayMicroseconds(STEP_DELAY);
  }

  delay(SENSOR_DELAY);

  rotateMotor(250, true);  // 180 degrees CW

  if (digitalRead(LEFT_SENSOR_PIN) == LOW) {
    delay(1000);
    digitalWrite(CW_PIN, HIGH);
    digitalWrite(CCW_PIN, LOW);
    delayMicroseconds(STEP_DELAY * 8);
    digitalWrite(CW_PIN, LOW);
    digitalWrite(CCW_PIN, LOW);
    delay(SENSOR_DELAY);
  }

  Serial.println("Home Base");
}

void loop() {
  if (Serial.available()) {
    char command = Serial.read();
    
    if (command == 'R') {
      resetMotor();
    } else {
      int angle = Serial.parseInt();
      bool clockwise = angle < 0;

      int steps = abs(angle) / STEP_ANGLE;
      rotateMotor(steps, clockwise);

      Serial.println("Completed: " + String(clockwise ? "CW" : "CCW") + ", " + String(abs(angle)));
    }
  }
}
