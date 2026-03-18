#include <ESP32PWM.h>
#include <Arduino.h>

// Servo constants
#define SERVO_PIN 0
#define SERVO_FREQUENCY 50
#define SERVO_RESOLUTION 12

#define SERVO_MAX_LEFT_PWM 404
#define SERVO_NEUTRAL_PWM 304
#define SERVO_MAX_RIGHT_PWM 204        

// ESC constants
#define ESC_PIN 1
#define ESC_FREQUENCY 50
#define ESC_RESOLUTION 14

#define ESC_MAX_FORWARD_PWM 1305        // Values have been carefully chosen from the actual range
#define ESC_MIN_FORWARD_PWM 1280        // It's crucial to limit the full power of the car
#define ESC_MAX_BACKWARD_PWM 1145
#define ESC_MIN_BACKWARD_PWM 1195
#define ESC_STOP_PWM 1255

// Other constants
#define BAUD_RATE 115200
#define CMD_DURATION 1000

// Global variables
ESP32PWM servo(false), esc(false);
int steering = SERVO_NEUTRAL_PWM, throttle = ESC_STOP_PWM;
unsigned long lastExecutedCmdMillis = 0;

// Functions
void setSteering(int ratio);
void setThrottle(int ratio);

void setup() {
  // Servo setup
  pinMode(SERVO_PIN, OUTPUT);
  servo.attachPin(SERVO_PIN, SERVO_FREQUENCY, SERVO_RESOLUTION);
  servo.write(SERVO_NEUTRAL_PWM);

  // ESC setup
  pinMode(ESC_PIN, OUTPUT);
  esc.attachPin(ESC_PIN, ESC_FREQUENCY, ESC_RESOLUTION);
  esc.write(ESC_STOP_PWM);

  // Communication setup
  Serial.begin(BAUD_RATE);
  delay(5000);
}

void loop() {
  if (Serial.available() >= 2) { // Adjust steering and throttle
    int steering_ratio = Serial.parseInt(); 
    int throttle_ratio = Serial.parseInt();

    setSteering(steering_ratio);
    setThrottle(throttle_ratio);

    lastExecutedCmdMillis = millis();
  }
  else { 
    // Stop, don't move
    if (millis() - lastExecutedCmdMillis > CMD_DURATION) {
      setSteering(0);
      setThrottle(0);
      lastExecutedCmdMillis = millis();
    }
  }

  // Update vehicle state
  servo.write(steering);
  esc.write(throttle);
}

void setSteering(int ratio) {
  ratio = constrain(ratio, -100, 100);
  steering = SERVO_NEUTRAL_PWM - ratio;
}

void setThrottle(int ratio) {
  ratio = constrain(ratio, -100, 100);

  if (ratio == 0) { // STOP
    throttle = ESC_STOP_PWM;
  }
  
  else if (ratio < 0) { // BACKWARD
    throttle = map(ratio, -100, -1, ESC_MAX_BACKWARD_PWM, ESC_MIN_BACKWARD_PWM);
  }

  else { // FORWARD
    throttle = map(ratio, 1, 100, ESC_MIN_FORWARD_PWM, ESC_MAX_FORWARD_PWM);
  }
}