#include <ArduinoJson.h>
#include <Servo.h>

// Servo constants
const int SERVO_PIN = 5, SERVO_L = 2100, SERVO_N = 1500, SERVO_R = 1020;

// ESC constants
const int ESC_PIN = 6, ESC_FWD = 1580,  ESC_STOP = 1495, ESC_BWD = 1410, ESC_BRAKE = 1125;

// Timing constants
const int  CMD_DURATION_MS = 10000, REV_STEP_TIME_MS = 150, BRAKE_DURATION_MS = 100;
const long BAUD_RATE = 115200;

// Global variables
Servo esc, servo;
unsigned long lastCmdMillis = 0;
int targetThrottle = 0; 
int lastTargetThrottle = 0;

// Non-blocking state machine for ESC
enum RevState { IDLE, BRAKE_TO_REV, UNLOCK, ENGAGE, ACTIVE_BRAKE };
RevState revSeq = IDLE;
unsigned long revTimer = 0;

void setSteering(int ratio);
void setThrottle(int ratio);
void updateMotors();

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
      if (doc.containsKey("steering")) setSteering(doc["steering"]);
      if (doc.containsKey("throttle")) setThrottle(doc["throttle"]);
      lastCmdMillis = millis();
    }
  }

  // 2. Failsafe
  if (millis() - lastCmdMillis > CMD_DURATION_MS) {
    setSteering(0);
    setThrottle(0);
  }

  // 3. Update the ESC state machine
  updateMotors();
}

void setSteering(int ratio) {
  ratio = constrain(ratio, -100, 100);
  int pwmValue;
  
  // Scenario I: Turn left
  if (ratio < 0) 
    pwmValue = map(ratio, -100, 0, SERVO_L, SERVO_N);
  
  // Scenario II: Turn right
  else 
    pwmValue = map(ratio, 0, 100, SERVO_N, SERVO_R);

  servo.writeMicroseconds(pwmValue);
}

void setThrottle(int ratio) {
  if (lastTargetThrottle == ratio) return;
  
  // Scenario I: Transitioning to reverse (double-tap sequence)
  if (ratio == -1 && lastTargetThrottle >= 0) {
    revSeq = BRAKE_TO_REV;
    revTimer = millis();
  } 

  // Scenario II: Transitioning from forward to stop (active Braking)
  else if (ratio == 0 && lastTargetThrottle == 1) {
    revSeq = ACTIVE_BRAKE;
    revTimer = millis();
  }
  
  // Scenario III: Any other change (like going 0 to 1 or -1 to 0)
  else if (ratio != -1 && revSeq != ACTIVE_BRAKE) {
    revSeq = IDLE;
  }
  
  targetThrottle = ratio;
  lastTargetThrottle = targetThrottle;
}

void updateMotors() {
  // Execute the logic based on the current state of the ESC state machine
  switch (revSeq) {

    // REVERSE SEQUENCE I: Brake and start reverse timer
    case BRAKE_TO_REV: 
      esc.writeMicroseconds(ESC_BWD); 
      if (millis() - revTimer >= REV_STEP_TIME_MS) {
        revSeq = UNLOCK; 
        revTimer = millis();
      }
      break;

    // REVERSE SEQUENCE II: Send neutral signal (required by most ESCs to engage reverse)
    case UNLOCK: 
      esc.writeMicroseconds(ESC_STOP);
      if (millis() - revTimer >= REV_STEP_TIME_MS) revSeq = ENGAGE;
      break;

    // REVERSE SEQUENCE III: Move backward
    case ENGAGE: 
      esc.writeMicroseconds(ESC_BWD); 
      break;

    // ACTIVE BRAKING: Send backward signal to brake
    case ACTIVE_BRAKE: 
      esc.writeMicroseconds(ESC_BRAKE);
      if (millis() - revTimer >= BRAKE_DURATION_MS) revSeq = IDLE;
      break;

    // NORMAL OPERATION: Send forward/stop signal 
    case IDLE: 
    default:
      if (targetThrottle == 1) esc.writeMicroseconds(ESC_FWD);
      else esc.writeMicroseconds(ESC_STOP);
      break;
  }
}