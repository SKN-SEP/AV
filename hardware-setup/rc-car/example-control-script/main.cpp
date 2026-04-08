#include "serialib.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdint>

// Constants
const char* DEVICE_NAME = "/dev/ttyUSB0";
const int BAUD_RATE = 115200;
const int DURATION_MS = 5000;
const int DELAY_MS = 20;

// Packed definition
struct CmdVelPacket {
    uint8_t header = 0xAA;
    int8_t throttle = 0;
    int8_t steering = 0;
    uint8_t checksum = 0;
};

int main() {
    // Open serial port
    std::cout << "============= START OF SWEEP TEST ============= \n";
    serialib my_serial;
    if (my_serial.openDevice(DEVICE_NAME, BAUD_RATE) != 1) {
        std::cerr << "[ERROR]: Error opening port: " << DEVICE_NAME << "\n";
        return 1;
    }
    std::cout << "[INFO]: Port opened successfully!\n";

    // Sweep logic setup
    int8_t current_steering = -100; 
    int8_t step_size = 1;
    int8_t target_steering = 100;

    std::cout << "[INFO]: Starting sweep from -100 to 100...\n";
    while (true) {

        // Create packet object
        CmdVelPacket p;
        p.throttle = 1;
        p.steering = (int8_t)current_steering;
        p.checksum = p.header ^ (uint8_t)p.steering ^ (uint8_t)p.throttle;
        
        // Send packet
        uint8_t buffer[4] = { p.header, (uint8_t)p.steering, (uint8_t)p.throttle, p.checksum };
        my_serial.writeBytes(buffer, 4);

        // Update the steering value 
        if (current_steering == target_steering) break; 
        current_steering += step_size;

        // Maintain the 10ms frequency
        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_MS));
    } 

    // Send stop packet: throttle 0, steering 0
    CmdVelPacket stop_p;
    uint8_t stop_buffer[4] = { 0xAA, 0, 0, 0xAA }; // header ^ 0 ^ 0 = header
    my_serial.writeBytes(stop_buffer, 4);
    my_serial.closeDevice();

    std::cout << "============= END OF THE TEST ============= \n";
    std::cout << "Final steering reached: " << (int)current_steering << "\n";
    return 0;
}