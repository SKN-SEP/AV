#include "serialib.h"
#include <iostream>
#include <random>
#include <chrono>
#include <thread>

// Constants
const char* DEVICE_NAME = "COM4"; // Ensure this matches your Device Manager
const int BAUD_RATE = 115200;
const int DURATION_MS = 5000;
const int DELAY_MS = 10;

struct CmdVelPacket {
    uint8_t header = 0xAA;
    int8_t throttle = 0;
    int8_t steering = 0;
    uint8_t checksum = 0;
};

int main() {
    std::cout << "============= START OF THE TEST ============= \n";
    serialib my_serial;

    // 1. Open Serial Port
    if (my_serial.openDevice(DEVICE_NAME, BAUD_RATE) != 1) {
        std::cerr << "[ERROR]: Could not open " << DEVICE_NAME << ". Check if it's in use!\n";
        return 1;
    }
    
    // 2. When the port opens, Arduino Nano Every resets. We must wait for it to boot.
    std::cout << "[INFO]: Port opened. Waiting 2s for Arduino to reboot..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Setup Randomness
    unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed);
    std::uniform_int_distribution<int> steering_dist(-100, 100);
    std::uniform_int_distribution<int> throttle_dist(-1, 1);

    std::cout << "[INFO]: Starting data transmission...\n";
    auto start_time = std::chrono::high_resolution_clock::now();
    int sent_packets = 0;
    int received_acks = 0;

    // 3. Main Loop
    while (std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::high_resolution_clock::now() - start_time).count() < DURATION_MS) {
        
        CmdVelPacket p;
        p.steering = (int8_t)steering_dist(generator);
        p.throttle = (int8_t)throttle_dist(generator);
        p.checksum = p.header ^ (uint8_t)p.steering ^ (uint8_t)p.throttle;
        
        uint8_t buffer[4] = { p.header, (uint8_t)p.steering, (uint8_t)p.throttle, p.checksum };
        
        // Write packet
        my_serial.writeBytes(buffer, 4);
        sent_packets++;

        // Read feedback (check for ACK from Arduino)
        // We use a small timeout to avoid blocking the loop too long
        char feedback;
        if (my_serial.readChar(&feedback, 5) == 1) { 
            received_acks++;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_MS));
        
        // Print progress every 50 packets so you know it's alive
        if (sent_packets % 50 == 0) {
            std::cout << "\r[PROGRESS]: Sent: " << sent_packets << " | ACKs: " << received_acks << std::flush;
        }
    } 

    my_serial.closeDevice();
    
    std::cout << "\n============= TEST COMPLETE ============= \n";
    std::cout << "Test duration: " << DURATION_MS << "ms\n";
    std::cout << "Total packets sent: " << sent_packets << '\n';
    std::cout << "Total ACKs received: " << received_acks << '\n';
    std::cout << "Success Rate: " << (float)received_acks / sent_packets * 100.0f << "%\n";

    return 0;
}