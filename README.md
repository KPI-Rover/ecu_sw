# KPI ROS2 Rover ECU Software
Welcome to the GitHub repository for the chassis controller software for the four-wheel rover as part of the KPI Rover robot project.

## Main Features:
1. **Microcontroller (MCU): STM32** The project utilizes an STM32 microcontroller to provide controller functionality.
2. **Motor Control:** The controller manages 4 DC motors in the four-wheel rover using L298N drivers.
3. **Encoder Integration:** For precise motion tracking, the controller reads signals from 4 encoders, measuring the rover’s speed and distance traveled.
4. **Battery Monitoring:** The controller reads and monitors the battery voltage level, allowing for timely responses to voltage changes or low battery warnings.
5. **ROS2 Communication:** To integrate the rover into the ROS2 system, the controller implements a communication protocol via a serial interface.

## KPI_Rover/Database

### Updating entries

In order to change database entries two steps are required:
- changing database metadata `ulDatabase_params` in `KPI_Rover/KPIRover.c`;
- updating the corresponding field names in the `ulDatabase_ParamId` enum in `KPI_Rover/Database/ulDatabase.h`.

To simplify this procedure the `KPI_Rover/Database/scripts/updateMetadataEnum.py` script is provided. It is required to fill out the comments after every metadata entry with the corresponding enum field name for the script to succeed. Setting specific CWD is *not* required, the script will find the files regardless.

The script has to be run after performing the first step mentioned above. It will read `ulDatabase_params` from `KPI_Rover/KPIRover.c` and validate it. If the check succeeds, the file `KPI_Rover/Database/ulDatabase.h` will be updated and the script will return zero. In case a validity check fails, the script will print the specific line that failed the check, underline the problematic value in the line and specify the rule the value has broken. The `KPI_Rover/Database/ulDatabase.h` file will remain unchanged and the exit code will be set to a non-zero value.
