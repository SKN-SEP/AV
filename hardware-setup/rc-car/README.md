# Process of setting up MAVERICK Quantum2 MT Flux

### Table of contents
- [Description](#description)
- [ESC FLX10-3S 80](#esc-flx10-3s-80)
- [Receiver MRX-400 2.4 GHz](#receiver-mrx-400-24-ghz)
- [Reverse Engineering of Control Signals](#reverse-engineering-of-control-signals)
- [Test sketches](#test-sketches)

## Description
The Maverick Quantum2 MT Flux 1/10 4WD Brushless Monster Truck is a ready-to-run (RTR) radio-controlled monster truck designed for off-road driving and high-speed “bashing.” It belongs to the Quantum2 RC series and features a strong 4-wheel-drive drivetrain and brushless power system for high performance.

<p align="center">
    <img src="./assets/rc-car.png" width="400">
</p>

## ESC FLX10-3S 80
The FLX10‑3S80 Flux is a brushless electronic speed controller (ESC) designed for 1:10 scale RC cars, especially those in the Maverick Quantum/Quantum2 series. It manages power delivery from the battery to the brushless motor and enables throttle, braking, and reverse control for dynamic driving performance.

<p align="center">
    <img src="./assets/esc.png" width="400">
</p>

### Key features
1. **Continuous Current**: 80 A — supports strong acceleration and high speeds.

2. **Input Voltage**: Works with 2S–3S LiPo batteries (7.4 V–11.1 V).

3. **BEC (Built‑in)**: 6 V switching BEC with ~2 A output to power servos/receiver.

4. **Connectors**: XT60 for battery and 4 mm bullet connectors for the motor (depends on version).

5. **Cooling**: Large onboard fan and heatsink help manage heat under load.

6. **Water Resistance**: Designed to tolerate splashes/mud for off‑road use.

## Receiver MRX-400 2.4 GHz
The MRX‑400 2.4 GHz is the radio receiver used in many Maverick Quantum2 series RC cars. It’s the component that sits inside the vehicle and receives the control signals from your radio transmitter, such as throttle and steering commands, then sends them to the ESC (speed controller) and servos.

<p align="center">
    <img src="./assets/receiver.png" width="400">
</p>

### Channel informations
1. **CH1 (Steering)**:Connects to the steering servo. Signals from the transmitter control the left/right steering of the vehicle.

2. **CH2 (Throttle / ESC)**: Connects to the Electronic Speed Controller (ESC). This channel sends throttle and brake commands from the transmitter. Supplys the power from ESC to the receiver (BEC).

3. **Pin Matrix**: the first row on left is signal, the second is voltage and the last is ground.

## Reverse Engineering of Control Signals
The goal is to understand how the RC interprets control signals from the transmitter and receiver by capturing the **PWM (pulse-width-modulation)** signals sent to the *ESC (throttle)* and *servo (steering)*. This allows us to analyze voltage levels and behaviour under different inputs.

### What is PWM?
Pulse-width modulation (PWM) is a technique used to control the average power delivered to an electrical load by varying the width of the pulses in a digital signal. It is commonly used in applications like motor control, LED dimming, and power regulation.

### RC PWM (FLX10 ESC)
It's a technical standard for electronic speed controlers (ESCs) which is different from the power-delivery PWM used for e.g. LEDs. Standard RC signals operate at a 50Hz frequency (a 20ms period). In this case pulse width differs from 1.0 ms to 2.0 ms. The signal should be 3.3V. We change the pulse width in order to manipulate the steering and throttle.

### Steering pulse widths
| Turn max left | No turning | Turn max right |
|-|-|-|
| <img src="./assets/ch1/turn-left.png" width="400"> | <img src="./assets/ch1/no-turn.png" width="400"> | <img src="./assets/ch1/turn-right.png" width="400">|
| <p align="center">2100 µs</p> | <p align="center">1500 µs</p> | <p align="center">1020 µs</p> |  

### Throttle pulse widths
| Move max forward | No move | Move max backward |
|-|-|-|
| <img src="./assets/ch2/move-forward.png" width="400"> | <img src="./assets/ch2/no-move.png" width="400"> | <img src="./assets/ch2/move-backward.png" width="400"> |
| <p align="center">1675 µs</p> | <p align="center">1495 µs</p> | <p align="center">1125 µs</p> |  

## Test sketches
Initial tests required from us to use microcontroller that can create output of 3.3V, in order to fulfill this requirement we used **ESP32-C3 Super Mini**. Additionally we needed a few cables and breadboard.

<p align="center">
    <img src="./assets/rc-car-test.jpg" width="400">
</p>

### Steering test ESP32
This test verifies the steering servo's range of motion and identifies the physical center point ("trim"). By utilizing a 50Hz PWM signal, we determine the specific microsecond limits that allow for maximum steering throw without causing mechanical binding or straining the linkage.

<p align="center">
    <img src="./assets/steering-test.gif" width="400">
</p>

### Throttle test ESP32
This test validates the Electronic Speed Controller (ESC) logic, specifically focusing on the "Double-Tap" safety sequence required to engage reverse gear on the Flux AC 80A. The goal is to synchronize the STOP_PWM and directional thresholds with the ESP32’s 14-bit resolution to ensure reliable transitions between braking, neutral, and backward motion.

<p align="center">
    <img src="./assets/throttle-test.gif" width="400">
</p>