#include <ArduinoJson.h>
#include <Servo.h>

// Servo constants
const int SERVO_PIN = 5, SERVO_L = 2100, SERVO_N = 1500, SERVO_R = 1020;

// ESC constants
const int ESC_PIN = 6, ESC_FWD = 1575, ESC_STOP = 1495, ESC_BWD = 1410;

// Timing constants
const int  CMD_DURATION_MS = 10000, BRAKE_DURATION_MS = 30, STOP_DURATION_MS = 30;
const long BAUD_RATE = 115200;

// Global variables
Servo esc, servo;
unsigned long lastCmdMillis = 0, brakingMillis = 0, stopMillis = 0;
enum ThrottleCmd { FORWARD, BACKWARD, STOP, BRAKING_FWD, BRAKING_BWD, STANDSTILL};
ThrottleCmd prevThrottleCmd = ThrottleCmd::STOP;
int steeringPulseWidth = SERVO_N, throttlePulseWidth = ESC_STOP;

void setSteering();
void setThrottle();

void setup() {
  // 1. Setup serial
  Serial.begin(BAUD_RATE);
  while (!Serial) {}

  // 2. Setup servo
  servo.attach(SERVO_PIN);
  servo.writeMicroseconds(SERVO_N);

  // 3. Setup ESC
  esc.attach(ESC_PIN);
  esc.writeMicroseconds(ESC_STOP);
}

void loop() {
  // 1. Check for input
  if (Serial.available() > 0) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, Serial);

    if (!error) {
      // Set new steering pulse width
      if (doc.containsKey("S")) {
        int steeringRatio = constrain((int) doc["S"], -100, 100);

        steeringPulseWidth = (steeringRatio < 0) ? 
          map(steeringRatio, -100, 0, SERVO_L, SERVO_N) :
          map(steeringRatio, 0, 100, SERVO_N, SERVO_R);
      }

      if (doc.containsKey("T")) {
        int throttleRatio = constrain((int) doc["T"], -100, 100);
        throttlePulseWidth = (throttleRatio < 0) ? 
          map(throttleRatio, -100, 0, ESC_BWD, ESC_STOP) :
          map(throttleRatio, 0, 100, ESC_STOP, ESC_FWD);
      }
      lastCmdMillis = millis();
    }
  }

  // 2. Failsafe
  if (millis() - lastCmdMillis > CMD_DURATION_MS) {
    steeringPulseWidth = SERVO_N;
    throttlePulseWidth = ESC_STOP;
  }
  
  setSteering();
  setThrottle();
}

void setSteering() {
  servo.writeMicroseconds(steeringPulseWidth);
}

void setThrottle() {

  // Find sign of the command
  switch ((throttlePulseWidth > ESC_STOP) - (throttlePulseWidth < ESC_STOP)) {

    // I. Go backward
    case -1: 
      switch (prevThrottleCmd) {
        // Run bracking procedure
        case ThrottleCmd::FORWARD:
        case ThrottleCmd::STOP:
          esc.writeMicroseconds(ESC_BWD);
          brakingMillis = millis();
          prevThrottleCmd = ThrottleCmd::BRAKING_FWD;
          break;
        
        // Execute braking procedure
        case ThrottleCmd::BRAKING_FWD:
          if (millis() - brakingMillis < BRAKE_DURATION_MS) {  
            esc.writeMicroseconds(ESC_BWD);
          }
          else {
            esc.writeMicroseconds(ESC_STOP);
            prevThrottleCmd = ThrottleCmd::STANDSTILL;
            stopMillis = millis();
          }
          break; 
        
        // Execute standstill procedure
        case ThrottleCmd::STANDSTILL:
          if (millis() - stopMillis < STOP_DURATION_MS) {
            esc.writeMicroseconds(ESC_STOP);
          }
          else {
            esc.writeMicroseconds(throttlePulseWidth);
            prevThrottleCmd = ThrottleCmd::BACKWARD;
          }
          break;
      }
      break;
    
    // II. Stop (Braking procedures)
    case 0:

      switch (prevThrottleCmd) {
        // Run braking procedure
        case ThrottleCmd::FORWARD:
          esc.writeMicroseconds(ESC_BWD);
          prevThrottleCmd = ThrottleCmd::BRAKING_FWD;
          brakingMillis = millis();
          break;
        
        // Execute forward braking procedure
        case ThrottleCmd::BRAKING_FWD:
          if (millis() - brakingMillis < BRAKE_DURATION_MS) {  
            esc.writeMicroseconds(ESC_BWD);
          }
          else {
            esc.writeMicroseconds(ESC_STOP);
            prevThrottleCmd = ThrottleCmd::STOP;
            stopMillis = millis();
          }
          break;
        
        // Run backward bracking procedure
        case ThrottleCmd::BACKWARD:
          esc.writeMicroseconds(ESC_STOP);
          prevThrottleCmd = ThrottleCmd::STOP;
          stopMillis = millis();
          break;
      }

      break;
    
    // III. Go forward
    case 1:
      esc.writeMicroseconds(throttlePulseWidth);
      prevThrottleCmd = ThrottleCmd::FORWARD;
      break;
  }
}