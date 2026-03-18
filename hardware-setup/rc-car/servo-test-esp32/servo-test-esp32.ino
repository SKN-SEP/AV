#include <ESP32PWM.h>

// PWM Settings
#define SERVO_PIN 0
#define SERVO_FREQUENCY 50
#define SERVO_RESOLUTION 12

#define SERVO_MAX_LEFT_PWM 404
#define SERVO_NEUTRAL_PWM 304
#define SERVO_MAX_RIGHT_PWM 204 

#define BAUD_RATE 115200

// Global variables
ESP32PWM servo(false);

void setup() {
  pinMode(SERVO_PIN, OUTPUT);
  servo.attachPin(SERVO_PIN, SERVO_FREQUENCY, SERVO_RESOLUTION);
  Serial.begin(BAUD_RATE);
}

void loop() {
  Serial.println("======= START OF THE TEST =======");

  // 1. Turn right
  Serial.println("First step: Turn right.");
  servo.write(SERVO_MAX_RIGHT_PWM);
  delay(1000);

  // 2. Straighten
  Serial.println("Second step: Straighten.");
  servo.write(SERVO_NEUTRAL_PWM);
  delay(1000);

  // 3. Turn left
  Serial.println("Third step: Turn left.");
  servo.write(SERVO_MAX_LEFT_PWM);
  delay(1000);

  // 4. Straighten
  Serial.println("Fourth step: Straighten.");
  servo.write(SERVO_NEUTRAL_PWM);
  delay(1000);

  Serial.println("======= END OF THE TEST =======");
  delay(3000);
}
