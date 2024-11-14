# KPI ROS2 Rover ECU Software
Welcome to the GitHub repository for the chassis controller software for the four-wheel rover as part of the KPI Rover robot project.

## Main Features:
1. **Microcontroller (MCU): STM32** The project utilizes an STM32 microcontroller to provide controller functionality.
2. **Motor Control:** The controller manages 4 DC motors in the four-wheel rover using L298N drivers.
3. **Encoder Integration:** For precise motion tracking, the controller reads signals from 4 encoders, measuring the rover’s speed and distance traveled.
4. **Battery Monitoring:** The controller reads and monitors the battery voltage level, allowing for timely responses to voltage changes or low battery warnings.
5. **ROS2 Communication:** To integrate the rover into the ROS2 system, the controller implements a communication protocol via a serial interface.

