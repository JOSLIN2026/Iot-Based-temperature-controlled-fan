# IoT-Based Temperature Controlled Fan

An IoT-based smart fan system that automatically adjusts fan speed using real-time temperature data.  
Built with ESP32, a DHT sensor, PWM fan control, an I2C LCD, and a web dashboard for remote monitoring and manual override.

## Short Description

This project combines embedded control and IoT monitoring to build a smart temperature-controlled fan.  
The ESP32 reads temperature data, adjusts fan speed automatically, and also allows manual control through a web interface.

## Problem Statement

Manual fan control is inefficient and does not adapt to changing environmental conditions.  
This can lead to unnecessary energy consumption, reduced comfort, and poor system responsiveness.

There is a need for an automated system that can:
- Adjust fan speed dynamically
- Provide remote monitoring and control
- Improve energy efficiency
- Switch between automatic and manual operation when needed

## Solution

This project implements an intelligent fan control system using an ESP32-based controller.

The system:
- Reads real-time temperature using a DHT sensor
- Automatically adjusts fan speed based on temperature thresholds
- Allows manual speed override using a web slider
- Displays live temperature and fan status on an I2C LCD
- Hosts a web server in Access Point mode for monitoring and control

## Features

- Real-time temperature monitoring using DHT22
- Automatic fan speed control based on temperature
- Manual fan speed override from web dashboard
- ESP32-hosted web interface for live monitoring
- I2C LCD display for temperature and fan status
- ESP32 Wi-Fi Access Point mode
- PWM-based fan speed regulation
- Auto/manual mode switching

## Components Used

- ESP32 development board
- DHT11 / DHT22 temperature sensor
- DC fan
- MOSFET / motor driver module
- I2C 16x2 LCD display
- Power supply / battery pack
- Breadboard
- Connecting wires

## Circuit Diagram

Prototype wiring / hardware view:

![Circuit / Wiring](images/wiring.jpg)

## Working Principle

1. The temperature sensor continuously measures ambient temperature.
2. The ESP32 reads the sensor data at regular intervals.
3. In auto mode, predefined temperature ranges decide the fan speed.
   Low temperature -> low fan speed.
   Medium temperature -> medium fan speed.
   High temperature -> high fan speed.
4. PWM output from the ESP32 controls the fan speed through a MOSFET or driver stage.
5. The ESP32 hosts a web server in Access Point mode.
   It displays live temperature, shows current fan speed, allows manual control through a slider, and lets the user switch back to auto mode.
6. The I2C LCD displays real-time temperature and fan status locally.

## Output / Results

- Successfully controlled fan speed based on temperature thresholds
- Achieved smooth fan speed variation using PWM
- Enabled real-time monitoring through the ESP32 web dashboard
- Added manual override without losing automatic control capability
- Displayed live temperature and fan speed on the LCD screen

## Demo

[Watch Demo Video](demo/smart-fan-demo.mp4)

## Project Images

### Final Setup

![Setup](images/setup.jpg)

### Internal Wiring and Prototype

![Working](images/wiring.jpg)

## Code

Code is available in this repository.

Main firmware file: `src/main.cpp`  
Project configuration: `platformio.ini`

## Challenges Faced

- Tuning PWM for smooth and stable fan speed control
- Ensuring reliable switching between auto mode and manual override
- Managing MOSFET or driver behavior so the fan receives enough power
- Integrating sensor reading, LCD updates, and web server responses without conflicts
- Designing a clean dashboard that updates in real time on ESP32 resources

## Future Improvements

- Mobile app integration
- Humidity-based control using full DHT22 data
- Data logging to cloud or local storage
- Energy usage analytics
- Smart home integration
- More advanced adaptive control using ML or prediction models

## Author

Joslin Joseph  
EEE Student | IoT & Embedded Systems Developer
