# GL ROS2 Rover ECU Software
Welcome to the GitHub repository for the Electronic Control Unit (ECU) software designed for the 4WD rover developed under the GL ROS2 Rover robot project. This project focuses on implementing firmware for the ECU, playing a crucial role in controlling various aspects of the rover's operation.

## Main Features:
**Microcontroller (MCU): STM32**
The project utilizes the STM32 microcontroller to drive the functionalities of the Electronic Control Unit.

**Motor Control:**
The ECU drives the 4 DC motors in the 4WD rover using L298N motor drivers, ensuring precise control and power distribution.

**Encoder Integration:**
To accurately track motion, the ECU reads signals from 4 encoders, measuring both speed and distance traveled by the rover.

**Battery Monitoring:**
The ECU reads and monitors the battery voltage level, enabling proactive measures in case of voltage fluctuations or low battery conditions.

**ROS2 Communication:**
To integrate the rover into the Robot Operating System 2 (ROS2) framework, the ECU implements a robust communication protocol over a serial interface. This facilitates data exchange and command reception, allowing the rover to be part of a larger ROS2-based robotic ecosystem.
