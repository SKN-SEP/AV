#include "serialib.h"
#include <random>
#include <chrono>
#include <thread>

// Constants
const char* DEVICE_NAME = "/dev/ttyUSB0";
const int BAUD_RATE = 115200;
const int DURATION_MS = 5000;
const int DELAY_MS = 10;
const int READ_TIMEOUT_MS = 10;

// Packed definition
struct CmdVelPacket {
    uint8_t header = 0xAA;
    int8_t throttle = 0;
    int8_t steering = 0;
    uint8_t checksum = 0;
};

int main() {
    // Open serial port
    std::cout << "============= START OF THE TEST ============= \n";
    serialib my_serial;
    if (my_serial.openDevice(DEVICE_NAME, BAUD_RATE) != 1) {
        std::cout << "============= END OF THE TEST ============= \n";
        std::cerr << "[ERROR]: Error opening port!\n";
        return 1;
    }
    std::cout << "[INFO]: Port opened successfully!\n";

    // Setup the random number generator
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed);
    std::uniform_int_distribution<int> steering_dist(-100, 100);
    std::uniform_int_distribution<int> throttle_dist(-1, 1);

    // Start sending data for 5 seconds
    std::cout << "[INFO]: Sending data\n";
    auto start_time = std::chrono::high_resolution_clock::now();
    int packet_counter = 0;

    while (std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - start_time).count() < DURATION_MS) {
        
        // Create packet object
        CmdVelPacket p;
        p.steering = steering_dist(generator);
        p.throttle = throttle_dist(generator);
        p.checksum = p.header ^ (uint8_t)p.steering ^ (uint8_t)p.throttle;
        
        // Send packet
        uint8_t buffer[4] = { p.header, (uint8_t)p.steering, (uint8_t)p.throttle, p.checksum };
        my_serial.writeBytes(buffer, 4);

        // Read feedback
        char r;
        if (my_serial.available() > 0) 
            if (my_serial.readChar(&r) == 1)
                packet_counter++;

        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_MS));
    } 
    
    // Calculate duration
    std::cout << "============= END OF THE TEST ============= \n";
    std::cout << "Test duration: " << DURATION_MS << "ms\n";
    std::cout << "Packet sending frequency: " << 1000.0 / (float) DELAY_MS << "Hz\n";
    std::cout << "Number of sent packets: " << packet_counter << '\n';

    return 0;
}