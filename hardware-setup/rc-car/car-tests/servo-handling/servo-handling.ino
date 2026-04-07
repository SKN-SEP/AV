#include <ESP32PWM.h>

// Servo settings
const int SERVO_PIN = 14, SERVO_FREQ = 50, SERVO_RES = 12;
const int SERVO_L = 390, SERVO_N = 305, SERVO_R = 220;

// Timing constants
const int BAUD_RATE = 115200, DELAY_MS = 1000;

// Global variables
ESP32PWM servo(false);

void setup() {
  pinMode(SERVO_PIN, OUTPUT);
  servo.attachPin(SERVO_PIN, SERVO_FREQ, SERVO_RES);
  Serial.begin(BAUD_RATE);
}

void loop() {
  Serial.println("======= START OF THE TEST =======");

  // 1. Turn right
  Serial.println("First step: Turn right.");
  servo.write(SERVO_R);
  delay(DELAY_MS);

  // 2. Straighten
  Serial.println("Second step: Straighten.");
  servo.write(SERVO_N);
  delay(DELAY_MS);

  // 3. Turn left
  Serial.println("Third step: Turn left.");
  servo.write(SERVO_L);
  delay(DELAY_MS);

  // 4. Straighten
  Serial.println("Fourth step: Straighten.");
  servo.write(SERVO_N);
  delay(DELAY_MS);

  Serial.println("======= END OF THE TEST =======");
  delay(3*DELAY_MS);
}
