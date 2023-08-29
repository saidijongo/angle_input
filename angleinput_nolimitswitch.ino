#include <Arduino.h>

const int CW_PIN = 5;
const int CCW_PIN = 6;
const int RIGHT_SENSOR_PIN = 11;
const int LEFT_SENSOR_PIN = 10;

const float STEP_ANGLE = 0.72;
const int SENSOR_DELAY = 1000;
const int STEP_DELAY = 10000;

bool motorStopped = true;
bool resetInProgress = false;
bool moveInProgress = false;
bool leftSensorInterrupted = false;
bool rightSensorInterrupted = false;

void setup() {
  pinMode(CW_PIN, OUTPUT);
  pinMode(CCW_PIN, OUTPUT);
  pinMode(RIGHT_SENSOR_PIN, INPUT_PULLUP);
  pinMode(LEFT_SENSOR_PIN, INPUT_PULLUP);
  Serial.begin(115200);
}

void rotateMotor(int steps, bool clockwise) {
  for (int i = 0; i < steps; i++) {
    if ((clockwise && leftSensorInterrupted) || (!clockwise && rightSensorInterrupted)) {
      stopMotor();
      return;
    }

    digitalWrite(clockwise ? CCW_PIN : CW_PIN, HIGH);
    delayMicroseconds(STEP_DELAY);
    digitalWrite(clockwise ? CCW_PIN : CW_PIN, LOW);
    delayMicroseconds(STEP_DELAY);
  }
}

void rotateToHomeBase() {
  while (digitalRead(RIGHT_SENSOR_PIN) == HIGH) {
    rotateMotor(1, false); // Rotate CCW until right sensor is interrupted
  }
  delay(1000); // Delay for stability

  while (digitalRead(LEFT_SENSOR_PIN) == HIGH) {
    rotateMotor(1, true); // Rotate CW until left sensor is interrupted
  }

  if (digitalRead(LEFT_SENSOR_PIN) == LOW) {
    rotateMotor(int(8 / STEP_ANGLE), true); // Rotate 8 degrees CW
    Serial.println("Home Base");
  }
}

void stopMotor() {
  digitalWrite(CW_PIN, LOW);
  digitalWrite(CCW_PIN, LOW);
  motorStopped = true;
  leftSensorInterrupted = false;
  rightSensorInterrupted = false;
  Serial.println("Motor Stopped");
}

void printStatus() {
  if (motorStopped) {
    if (resetInProgress) {
      Serial.println("Stopped: Home Base");
    } else {
      Serial.println("Stopped: Not Home Base");
    }
  } else {
    Serial.println("Rotating");
  }
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (!resetInProgress && !moveInProgress) {
      if (command == "C") {
        if (!leftSensorInterrupted) {
          motorStopped = false;
          moveInProgress = true;
          int targetSteps = int(90 / STEP_ANGLE);
          rotateMotor(targetSteps, true); // Rotate 90 degrees clockwise
          moveInProgress = false;
          motorStopped = true;
        }
      } else if (command == "D") {
        if (!rightSensorInterrupted) {
          motorStopped = false;
          moveInProgress = true;
          int targetSteps = int(90 / STEP_ANGLE);
          rotateMotor(targetSteps, false); // Rotate 90 degrees counter-clockwise
          moveInProgress = false;
          motorStopped = true;
        }
      } else if (command == "R") {
        resetInProgress = true;
        motorStopped = false;
        rotateToHomeBase();
        resetInProgress = false;
        motorStopped = true;
      } else if (command == "S") {
        stopMotor();
      } else if (command == "I") {
        printStatus();
      } else {
        int angle = command.toInt();
        bool clockwise = angle < 0;
        int steps = abs(angle) / STEP_ANGLE;
        rotateMotor(steps, clockwise);
      }
    }
  }
}
