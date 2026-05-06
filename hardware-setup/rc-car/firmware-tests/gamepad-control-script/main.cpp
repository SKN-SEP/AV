#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <SDL2/SDL.h>
#include "serialib.h"

// Cross-platform Serial Port selection
#if defined(_WIN32) || defined(_WIN64)
    const char* DEVICE_NAME = "COM4";
#else
    const char* DEVICE_NAME = "/dev/ttyACM0";
#endif

// Xbox Controller Constants
const int DEADZONE = 8000;
const int BAUD_RATE = 115200;
const int UPDATE_MS = 10;

void sendPacket(serialib &serial, int8_t throttle, int8_t steering) {
    uint8_t head = 0xAA;
    uint8_t crc = head ^ (uint8_t)throttle ^ (uint8_t)steering;
    uint8_t buf[4] = { head, (uint8_t)throttle, (uint8_t)steering, crc };
    
    serial.writeBytes(buf, 4);

    // Clean the "ACK" bytes from Arduino to keep buffers fresh
    while (serial.available() > 0) {
        char dummy;
        serial.readChar(&dummy);
    }
}

int main(int argc, char* argv[]) {
    serialib my_serial;
    
    // Initialize SDL2 GameController Subsystem
    if (SDL_Init(SDL_INIT_GAMECONTROLLER) < 0) {
        std::cerr << "SDL Init Failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Open Serial Port
    if (my_serial.openDevice(DEVICE_NAME, BAUD_RATE) != 1) {
        std::cerr << "Serial Error: Could not open " << DEVICE_NAME << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_GameController* controller = nullptr;
    std::cout << "Waiting for Xbox Controller..." << std::endl;

    bool running = true;
    SDL_Event e;

    while (running) {
        // Handle Controller Connection/Disconnection Events
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) running = false;
            
            if (e.type == SDL_CONTROLLERDEVICEADDED) {
                if (!controller) {
                    controller = SDL_GameControllerOpen(e.cdevice.which);
                    std::cout << "\n[CONNECTED] " << SDL_GameControllerName(controller) << std::endl;
                    std::cout << "Controls: Left Stick (Throttle), Right Stick (Steering)" << std::endl;
                    std::cout << "Press 'B' or 'Circle' to Exit." << std::endl;
                }
            }

            if (e.type == SDL_CONTROLLERDEVICEREMOVED) {
                std::cout << "\n[DISCONNECTED] Controller removed." << std::endl;
                SDL_GameControllerClose(controller);
                controller = nullptr;
            }

            if (e.type == SDL_CONTROLLERBUTTONDOWN) {
                if (e.cbutton.button == SDL_CONTROLLER_BUTTON_B) running = false;
            }
        }

        // Read Axis and Send Data
        if (controller) {
            // SDL Y-axis is -32768 (Up) to 32767 (Down). We invert it for our throttle.
            int16_t yVal = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
            int16_t xVal = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX);

            int8_t throttle = 0;
            int8_t steering = 0;

            if (std::abs(yVal) > DEADZONE) {
                throttle = (int8_t)((yVal / -32767.0f) * 100) / 2;
            }

            if (std::abs(xVal) > DEADZONE) {
                steering = (int8_t)((xVal / 32767.0f) * 100);
            }

            sendPacket(my_serial, throttle, steering);
            
            // Console feedback
            std::cout << "\rThrottle: " << (int)throttle << "% | Steering: " << (int)steering << "%    " << std::flush;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(UPDATE_MS));
    }

    // Cleanup and stop car
    std::cout << "\nShutting down..." << std::endl;
    sendPacket(my_serial, 0, 0);
    if (controller) SDL_GameControllerClose(controller);
    my_serial.closeDevice();
    SDL_Quit();

    return 0;
}