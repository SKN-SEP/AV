#include "serialib.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <string>

// Port configuration
#if defined(_WIN32) || defined(_WIN64)
    const char* DEVICE_NAME = "COM4"; // Windows port
#else
    const char* DEVICE_NAME = "/dev/ttyACM0"; // Linux/Jetson port
#endif

const int BAUD_RATE = 115200;


/**
 * Sends a 4-byte packet: [Header, Throttle, Steering, Checksum]
 * Also clears the incoming serial buffer to prevent Windows/Linux hangs.
 */
void sendPacket(serialib &serial, int8_t throttle, int8_t steering) {
    uint8_t head = 0xAA;
    uint8_t crc = head ^ (uint8_t)throttle ^ (uint8_t)steering;
    uint8_t buf[4] = { head, (uint8_t)throttle, (uint8_t)steering, crc };
    
    serial.writeBytes(buf, 4);

    // Drain ACK bytes ('A') from Arduino to keep the RX buffer empty
    while (serial.available() > 0) {
        char dummy;
        serial.readChar(&dummy);
    }
}

/**
 * Runs a specific throttle/steering command for a set duration.
 */
void runTest(serialib &serial, int8_t throttle, int8_t steering, int durationMs, std::string label) {
    std::cout << ">> Action: " << label << " (T:" << (int)throttle << " S:" << (int)steering << ")" << std::endl;
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() < durationMs) {
        sendPacket(serial, throttle, steering);
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 100Hz Update Rate
    }
}

/**
 * Main test sequence.
 */
int main() {
    serialib my_serial;

    std::cout << "==========================================" << std::endl;
    std::cout << "  ROBOT STRESS TEST: DIRECTIONAL LOGIC    " << std::endl;
    std::cout << "==========================================" << std::endl;

    std::cout << "Connecting to " << DEVICE_NAME << "..." << std::endl;
    if (my_serial.openDevice(DEVICE_NAME, BAUD_RATE) != 1) {
        std::cerr << "[!] ERROR: Could not open port. Check connection/permissions." << std::endl;
        return 1;
    }

    try {
        // --- TEST 1: THE RECIPROCAL SHUTTLE ---
        // Checks FWD -> BRAKE -> NEUTRAL -> BWD transition
        std::cout << "\n[TEST 1] SHUTTLE RUN (Forward to Backward)" << std::endl;
        runTest(my_serial, 60, 0, 1500, "Moving Forward");
        runTest(my_serial, -60, 0, 2000, "Engaging Reverse (Testing Double-Tap)");
        runTest(my_serial, 0, 0, 500, "Neutral Reset");

        // --- TEST 2: REVERSE SLALOM ---
        // Tests steering response while in backward state
        std::cout << "\n[TEST 2] REVERSE SLALOM (Steering while in BWD)" << std::endl;
        runTest(my_serial, -50, -100, 1000, "Reverse Hard Left");
        runTest(my_serial, -50, 100, 1000, "Reverse Hard Right");
        runTest(my_serial, 50, 0, 1000, "Instant Forward Recovery");

        // --- TEST 3: DIRECTIONAL STRESS (Rapid Switching) ---
        // Mimics a line follower hunting for a line by jittering direction
        std::cout << "\n[TEST 3] DIRECTIONAL STRESS (The Panic Loop)" << std::endl;
        for (int i = 0; i < 3; i++) {
            runTest(my_serial, 75, 0, 600, "Forward Burst");
            runTest(my_serial, -75, 0, 600, "Reverse Request");
        }

        // --- TEST 4: THROTTLE RAMPING ---
        // Checks linearity and dead-zones
        std::cout << "\n[TEST 4] FULL RAMP (-100 to 100)" << std::endl;
        for (int t = -100; t <= 100; t += 10) {
            runTest(my_serial, (int8_t)t, 0, 200, "Ramping Throttle");
        }

    } catch (...) {
        std::cerr << "An unexpected error occurred during testing." << std::endl;
    }

    // --- CLEANUP ---
    std::cout << "\n==========================================" << std::endl;
    std::cout << "TESTING COMPLETE. STOPPING MOTORS." << std::endl;
    sendPacket(my_serial, 0, 0);
    my_serial.closeDevice();

    // Nuclear exit for Windows stability (prevents port hanging)
    #if defined(_WIN32) || defined(_WIN64)
        std::exit(0);
    #endif

    return 0;
}