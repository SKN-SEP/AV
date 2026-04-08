#include <ESP32PWM.h>

// Packet Constants
const uint8_t HEADER = 0xAA;
const int PACKET_SIZE = 4;

// Servo constants
const int SERVO_PIN = 14, SERVO_FREQ = 50, SERVO_RES = 12;
const int SERVO_L = 390, SERVO_N = 305, SERVO_R = 220;

// ESC constants
const int ESC_PIN = 13, ESC_FREQ = 50, ESC_RES = 14;
const int ESC_FWD = 1290, ESC_STOP = 1255, ESC_BWD = 1140;

// Timing constants
const long BAUD_RATE = 115200;
const int CMD_DURATION_MS = 500;
const int REV_STEP_TIME_MS = 150;
const int BRAKE_DURATION_MS = 150;

// Global variables
ESP32PWM servo(false), esc(false);
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

  // 2. Setup servo
  servo.attachPin(SERVO_PIN, SERVO_FREQ, SERVO_RES);
  servo.write(SERVO_N);

  // 3. Setup ESC
  esc.attachPin(ESC_PIN, ESC_FREQ, ESC_RES);
  esc.write(ESC_STOP);
}

void loop() {
  // 1. Binary Packet Parsing (Silent)
  if (Serial.available() >= PACKET_SIZE) {
    uint8_t header = Serial.read();

    if (header == HEADER) {
      int8_t steeringRaw = (int8_t)Serial.read();
      int8_t throttleRaw = (int8_t)Serial.read();
      uint8_t checksum = Serial.read();

      // Verify integrity (XOR check)
      if (checksum == (header ^ (uint8_t)steeringRaw ^ (uint8_t)throttleRaw)) {
        setSteering((int)steeringRaw);
        setThrottle((int)throttleRaw);
        lastCmdMillis = millis();
      }
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

  servo.write(pwmValue);
}

void setThrottle(int ratio) {
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
  lastTargetThrottle = ratio;
}


void updateMotors() {
  // Execute the logic based on the current state of the ESC state machine
  switch (revSeq) {

    // REVERSE SEQUENCE I: Brake and start reverse timer
    case BRAKE_TO_REV: 
      esc.write(ESC_BWD); 
      if (millis() - revTimer >= REV_STEP_TIME_MS) {
        revSeq = UNLOCK; 
        revTimer = millis();
      }
      break;

    // REVERSE SEQUENCE II: Send neutral signal (required by most ESCs to engage reverse)
    case UNLOCK: 
      esc.write(ESC_STOP);
      if (millis() - revTimer >= REV_STEP_TIME_MS) revSeq = ENGAGE;
      break;

    // REVERSE SEQUENCE III: Move backward
    case ENGAGE: 
      esc.write(ESC_BWD); 
      break;

    // ACTIVE BRAKING: Send backward signal to brake
    case ACTIVE_BRAKE: 
      esc.write(ESC_BWD);
      if (millis() - revTimer >= BRAKE_DURATION_MS) revSeq = IDLE;
      break;

    // NORMAL OPERATION: Send forward/stop signal 
    case IDLE: 
    default:
      if (targetThrottle == 1) esc.write(ESC_FWD);
      else esc.write(ESC_STOP); 
      break;
  }
}
  
  // Scenario III: Any other change (like going 0 to 1 or -1 to 0)
  else if (ratio != -1 && revSeq != ACTIVE_BRAKE) {
    revSeq = IDLE;
  }
  
  targetThrottle = ratio;
  lastTargetThrottle = ratio;
}


void updateMotors() {
  // Execute the logic based on the current state of the ESC state machine
  switch (revSeq) {

    // REVERSE SEQUENCE I: Brake and start reverse timer
    case BRAKE_TO_REV: 
      esc.write(ESC_BWD); 
      if (millis() - revTimer >= REV_STEP_TIME_MS) {
        revSeq = UNLOCK; 
        revTimer = millis();
      }
      break;

    // REVERSE SEQUENCE II: Send neutral signal (required by most ESCs to engage reverse)
    case UNLOCK: 
      esc.write(ESC_STOP);
      if (millis() - revTimer >= REV_STEP_TIME_MS) revSeq = ENGAGE;
      break;

    // REVERSE SEQUENCE III: Move backward
    case ENGAGE: 
      esc.write(ESC_BWD); 
      break;

    // ACTIVE BRAKING: Send backward signal to brake
    case ACTIVE_BRAKE: 
      esc.write(ESC_BWD);
      if (millis() - revTimer >= BRAKE_DURATION_MS) revSeq = IDLE;
      break;

    // NORMAL OPERATION: Send forward/stop signal 
    case IDLE: 
    default:
      if (targetThrottle == 1) esc.write(ESC_FWD);
      else esc.write(ESC_STOP); 
      break;
  }
}
