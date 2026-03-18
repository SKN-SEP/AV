#include <ESP32PWM.h>

// PWM Settings
#define ESC_PIN 1
#define ESC_FREQUENCY 50
#define ESC_RESOLUTION 14

// PWM Values
#define ESC_MAX_FORWARD_PWM 1305        // Values have been carefully chosen from the actual range
#define ESC_MIN_FORWARD_PWM 1280        // It's crucial to limit the full power of the car
#define ESC_MAX_BACKWARD_PWM 1145
#define ESC_MIN_BACKWARD_PWM 1195
#define ESC_STOP_PWM 1255

#define BAUD_RATE 115200

// Global variables
ESP32PWM esc(false);

void stop(int time, int pwm);

void setup() {
  pinMode(ESC_PIN, OUTPUT);
  esc.attachPin(ESC_PIN, ESC_FREQUENCY, ESC_RESOLUTION);
  Serial.begin(BAUD_RATE);
}

void loop() {
  Serial.println("======= START OF THE TEST =======");

  // 1. Move forward
  Serial.println("First step: Moving forward.");
  esc.write(ESC_MAX_FORWARD_PWM);
  delay(2000);

  // 2. Braking
  Serial.println("Second step: Braking (backward for one second).");
  esc.write(ESC_MAX_BACKWARD_PWM);
  delay(150);

  // 3. Neutral
  Serial.println("Third step: Fooling ESC (unlocking backward movement).");
  esc.write(ESC_STOP_PWM);
  delay(150);

  // 4. Move backward 
  Serial.println("Fourth step: Moving backward.");
  esc.write(ESC_MAX_BACKWARD_PWM);
  delay(2000);

  // 5. Stop
  Serial.println("Fifth step: Stop.");
  esc.write(ESC_STOP_PWM);
  delay(1500);

  Serial.println("======= END OF THE TEST =======");
  delay(3000);
}
