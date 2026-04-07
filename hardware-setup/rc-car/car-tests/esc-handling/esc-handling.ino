#include <ESP32PWM.h>

// ESC settings
const int ESC_PIN = 13, ESC_FREQ = 50, ESC_RES = 14;
const int ESC_FWD = 1290,  ESC_STOP = 1255, ESC_BWD = 1140;

// Servo settings
const int SERVO_PIN = 14, SERVO_FREQ = 50, SERVO_RES = 12, SERVO_N = 305;

// Timing constants
const int BAUD_RATE = 115200, DELAY_MS = 1000;

// Global variables
ESP32PWM esc(false), servo(false);

void setup() {
  pinMode(ESC_PIN, OUTPUT);
  esc.attachPin(ESC_PIN, ESC_FREQ, ESC_RES);
  Serial.begin(BAUD_RATE);

  pinMode(SERVO_PIN, OUTPUT);
  servo.attachPin(SERVO_PIN, SERVO_FREQ, SERVO_RES);
}

void loop() {
  Serial.println("======= START OF THE TEST =======");
  servo.write(SERVO_N);
  delay(DELAY_MS);

  // 1. Move forward
  Serial.println("First step: Moving forward.");
  esc.write(ESC_FWD);
  delay(2*DELAY_MS);

  // 2. Braking
  Serial.println("Second step: Braking (backward for one second).");
  esc.write(ESC_BWD);
  delay(150);

  // 3. Neutral
  Serial.println("Third step: Fooling ESC (unlocking backward movement).");
  esc.write(ESC_STOP);
  delay(150);

  // 4. Move backward 
  Serial.println("Fourth step: Moving backward.");
  esc.write(ESC_BWD);
  delay(2*DELAY_MS);

  // 5. Stop
  Serial.println("Fifth step: Stop.");
  esc.write(ESC_STOP);
  delay(2*DELAY_MS);

  Serial.println("======= END OF THE TEST =======");
  delay(3*DELAY_MS);
}
