#include <Servo.h>

// Servo settings
const int SERVO_PIN = 5, SERVO_L = 2100, SERVO_N = 1500, SERVO_R = 1020;

// Timing constants
const int BAUD_RATE = 115200, DELAY_MS = 1000;

// Global variables
Servo servo;

void setup() {
  pinMode(SERVO_PIN, OUTPUT);
  servo.attach(SERVO_PIN);
  Serial.begin(BAUD_RATE);
}

void loop() {
  Serial.println("======= START OF THE TEST =======");

  // 1. Turn right
  Serial.println("First step: Turn right.");
  servo.writeMicroseconds(SERVO_R);
  delay(DELAY_MS);

  // 2. Straighten
  Serial.println("Second step: Straighten.");
  servo.writeMicroseconds(SERVO_N);
  delay(DELAY_MS);

  // 3. Turn left
  Serial.println("Third step: Turn left.");
  servo.writeMicroseconds(SERVO_L);
  delay(DELAY_MS);

  // 4. Straighten
  Serial.println("Fourth step: Straighten.");
  servo.writeMicroseconds(SERVO_N);
  delay(DELAY_MS);

  Serial.println("======= END OF THE TEST =======");
  delay(3*DELAY_MS);
}
