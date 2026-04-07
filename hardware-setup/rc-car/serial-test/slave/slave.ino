// Constants
const unsigned long BAUD_RATE = 115200;
const int PACKET_SIZE = 4;

void setup() {
  Serial.begin(BAUD_RATE);
}

void loop() {
  // Read packet
  if (Serial.available() >= PACKET_SIZE) {
    uint8_t header = Serial.read();
    
    // Extract packet data
    if (header == 0xAA) {
      int8_t throttle = Serial.read();
      int8_t steering = Serial.read();
      uint8_t checksum = Serial.read();

      // Check packet state and send feedback
      if (checksum == (header ^ (uint8_t)throttle ^ (uint8_t)steering))
        Serial.write(1);
      else
        Serial.write(0);
    }
  }
}
