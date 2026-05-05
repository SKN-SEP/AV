#include <Servo.h>

// ESC settings
const int ESC_PIN = 6, ESC_FWD = 1580,  ESC_STOP = 1495, ESC_BWD = 1395;

// Servo settings
const int SERVO_PIN = 5, SERVO_N = 1500;

// Timing constants
const int BAUD_RATE = 115200, DELAY_MS = 1000;

// Global variables
Servo esc, servo;

void setup() {
  pinMode(ESC_PIN, OUTPUT);
  esc.attach(ESC_PIN);
  Serial.begin(BAUD_RATE);

  pinMode(SERVO_PIN, OUTPUT);
  servo.attach(SERVO_PIN);
}

void loop() {
  Serial.println("======= START OF THE TEST =======");
  servo.writeMicroseconds(SERVO_N);
  delay(DELAY_MS);

  // 1. Move forward
  Serial.println("First step: Moving forward.");
  esc.writeMicroseconds(ESC_FWD);
  delay(2*DELAY_MS);

  // 2. Braking
  Serial.println("Second step: Braking (backward for one second).");
  esc.writeMicroseconds(ESC_BWD);
  delay(150);

  // 3. Neutral
  Serial.println("Third step: Fooling ESC (unlocking backward movement).");
  esc.writeMicroseconds(ESC_STOP);
  delay(150);

  // 4. Move backward 
  Serial.println("Fourth step: Moving backward.");
  esc.writeMicroseconds(ESC_BWD);
  delay(2*DELAY_MS);

  // 5. Stop
  Serial.println("Fifth step: Stop.");
  esc.writeMicroseconds(ESC_STOP);
  delay(2*DELAY_MS);

  Serial.println("======= END OF THE TEST =======");
  delay(3*DELAY_MS);
}
