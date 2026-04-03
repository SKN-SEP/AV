#include <ArduinoJson.h>
#include <ESP32PWM.h>

// Servo constants
#define SERVO_PIN 14
#define SERVO_FREQUENCY 50
#define SERVO_RESOLUTION 12

#define SERVO_MAX_LEFT_PWM 390
#define SERVO_NEUTRAL_PWM 305
#define SERVO_MAX_RIGHT_PWM 220

// ESC constants
#define ESC_PIN 13
#define ESC_FREQUENCY 50
#define ESC_RESOLUTION 14

#define ESC_FORWARD_PWM 1295  
#define ESC_BACKWARD_PWM 1145
#define ESC_STOP_PWM 1255

// Other constants
#define BAUD_RATE 115200
#define CMD_DURATION 2500

// Global variables
ESP32PWM servo(false), esc(false);
unsigned long lastCmdVelMillis = 0;
int lastThrottleRatio = 0;

// Functions
void setSteering(const int& ratio);
void setThrottle(const int& ratio);

void setup() {
  // Serial configuration
  Serial.begin(BAUD_RATE);
  while (!Serial) {}

  // Servo setup
  pinMode(SERVO_PIN, OUTPUT);
  servo.attachPin(SERVO_PIN, SERVO_FREQUENCY, SERVO_RESOLUTION);
  servo.write(SERVO_NEUTRAL_PWM);

  // ESC setup
  pinMode(ESC_PIN, OUTPUT);
  esc.attachPin(ESC_PIN, ESC_FREQUENCY, ESC_RESOLUTION);
  esc.write(ESC_STOP_PWM);
}

void loop() {
  // New message incoming
  if (Serial.available() > 0) {
    // Construct JSON
    String input = Serial.readStringUntil('\n');
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, input);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.f_str());
      return;
    }

    if (doc.containsKey("steering")) {
      Serial.println("Steering arg exists!");
      setSteering(doc["steering"]);
      lastCmdVelMillis = millis();
    }
    
    if (doc.containsKey("throttle")) {
      Serial.println("Throttle arg exists!");
      setThrottle(doc["throttle"]);
      lastCmdVelMillis = millis();
    }
  }

  // No input from master 
  if (millis() - lastCmdVelMillis > CMD_DURATION) {
    setSteering(0);
    setThrottle(0);
  }
}

void setSteering(const int& ratio) {
  // Constrain input to ensure it stays within -100 to 100
  int constrainedRatio = constrain(ratio, -100, 100);
  int pwmValue;

  if (constrainedRatio < 0) {
    // Map -100 to 0 (Left) to PWM range [390, 305]
    pwmValue = map(constrainedRatio, -100, 0, SERVO_MAX_LEFT_PWM, SERVO_NEUTRAL_PWM);
  } else {
    // Map 0 to 100 (Right) to PWM range [305, 220]
    pwmValue = map(constrainedRatio, 0, 100, SERVO_NEUTRAL_PWM, SERVO_MAX_RIGHT_PWM);
  }

  servo.write(pwmValue);
}

void setThrottle(const int& ratio) {
  // If moving from Forward (1) or Neutral (0) to Reverse (-1)
  if (ratio == -1 && lastThrottleRatio >= 0) {
    // 1. Brake
    esc.write(ESC_BACKWARD_PWM);
    delay(150);
    // 2. Unlock Reverse (Neutral)
    esc.write(ESC_STOP_PWM);
    delay(150);
    // 3. Engage Reverse
    esc.write(ESC_BACKWARD_PWM);
  }
  else if (ratio == 1) {
    esc.write(ESC_FORWARD_PWM);
  } 
  else if (ratio == 0) {
    esc.write(ESC_STOP_PWM);
  }

  // Update state for the next call
  lastThrottleRatio = ratio; 
}