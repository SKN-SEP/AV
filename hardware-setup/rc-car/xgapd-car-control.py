import pygame
import pygame.joystick as joystick
import serial
import time
import math

# UART constants
ESP_COM_PORT = 'COM5'
BAUD_RATE = 52000

# Joystick constants
GAMEPAD_ID = 0
LEFT_STICK_X_ID = 0
LEFT_STICK_Y_ID = 1

def send_values(ser, steering, throttle):
    message = f"{steering} {throttle}\n"
    ser.write(message.encode())
    ser.flush()
    time.sleep(0.000001)

def main():
    try:
        pygame.init()
        print("Number of connected joysticks:", joystick.get_count())
        gpad = joystick.Joystick(GAMEPAD_ID)
        ser = serial.Serial(ESP_COM_PORT, BAUD_RATE, timeout=0.1)

        
        while True:
            pygame.event.get()
            steering = math.floor(gpad.get_axis(LEFT_STICK_X_ID) * 100)
            throttle = math.floor(gpad.get_axis(LEFT_STICK_Y_ID) * -100)
            print("=====================")
            print("Steering:", steering)
            print("Throttle:", throttle)
        
        #for i in range(-100, 100, 1):
            send_values(ser, steering, throttle)
            time.sleep(0.001)

    except KeyboardInterrupt:
        print("Closing serial...")
        ser.close()

if __name__ == "__main__":
    main()
