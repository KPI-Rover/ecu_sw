# Software Architecture Design - Chassis Controller (STM32)
<!-- The markdown-toc utilitity is used to generate the Table of Contents -->
<!-- Installation: npm install -g markdown-toc -->
<!-- Usage: markdown-toc --maxdepth 2 -i docs/sad.md -->

<!-- toc -->

- [Software Structure](#software-structure)
- [General Architecture Decisions](#general-architecture-decisions)
- [Database](#database)
- [Communication Hub](#communication-hub)
- [MotorsController](#motorscontroller)
- [ADC Module](#adc-module)
- [IMU Module](#imu-module)
- [Compass Module](#compass-module)
- [GPS Module](#gps-module)
- [Encoders Module](#encoders-module)
- [LEDsController](#ledscontroller)
- [BuzzerController](#buzzercontroller)
- [Logger](#logger)

<!-- tocstop -->

**How to View PlantUML Diagrams**

To view PlantUML diagrams in this document:

1. Install **Visual Studio Code** with the **PlantUML** extension from: [https://marketplace.visualstudio.com/items?itemName=jebbs.plantuml](https://marketplace.visualstudio.com/items?itemName=jebbs.plantuml)
2. Configure the extension to use the PlantUML online render server:
   - Open VS Code settings (File → Preferences → Settings)
   - Search for "PlantUML: Server"
   - Set the server URL to: `https://www.plantuml.com/plantuml`
3. Open this Markdown file in VS Code.
4. Use the Markdown preview feature to view rendered diagrams (Ctrl+Shift+V or right-click → "Open Preview").

Alternatively, you can use the online PlantUML editor at [https://www.plantuml.com/plantuml](https://www.plantuml.com/plantuml) by copying the diagram code.

## Software Structure

```plantuml
    @startuml
    skinparam component {
    BackgroundColor<<communication>> LightBlue
    BackgroundColor<<sensor>> LightYellow
    BackgroundColor<<configuration>> LightGreen
    BackgroundColor<<control>> Orange
    BorderColor Black
    FontColor Black
    }

    component "Database" <<configuration>>

    component "Communication Hub" <<communication>>

    component "MotorsController" <<control>>
    
    component "ADC Module" <<sensor>>
    component "IMU Module" <<sensor>>
    component "Compass Module" <<sensor>>
    component "GPS Module" <<sensor>>
    component "Encoders Module" <<sensor>>

    component "LEDsController" <<control>>
    component "BuzzerController" <<control>>
    component "Logger"
    component "FeeRTOS"

    note top of "Logger"
      Used by all components
    end note

    [Communication Hub] .right.> [Database] : "use"
    
    [ADC Module] .down.> [Database] : "use"
    [IMU Module] .down.> [Database] : "use"
    [Compass Module] .down.> [Database] : "use"
    [GPS Module] .down.> [Database] : "use"
    [Encoders Module] .down.> [Database] : "use"
    [MotorsController] .up.> [Database] : "use"
    [LEDsController] .up.> [Database] : "use"
    [BuzzerController] .up.> [Database] : "use"
    @enduml
```

- **Database**: Central configuration and state store used by all components to read settings and publish state/telemetry.
- **Communication Hub**: Manages external communication interfaces.
- **MotorsController**: Control motors speed.
- **ADC Module**: Samples analog channels (battery, sensors) and writes measured values to the Database.
- **IMU Module**: Provides accelerometer and gyroscope data.
- **Compass Module**: Provides heading information and writes orientation data.
- **GPS Module**: Provide GPS data.
- **Encoders Module**: Reads wheel encoder counts, calculate speed.
- **LEDsController**: Controls visual indicators (LEDs).
- **BuzzerController**: Controls audible alerts.
- **Logger**: Provides logging output for all components.

## General Architecture Decisions

### SAD-D-1: Avoid direct communication between modules.
Communication between modules should be done using the Database to avoid tight coupling and hidden dependencies.

**Rules:**
- Modules MUST NOT call each other's internal APIs or share memory for coordination.
- All inter-module signals, commands and state MUST be published to and consumed from the Database using defined keys/topics and formats.

**Exceptions:**
- A limited, documented exception exists for hard real-time control loops where Database-mediated communication cannot meet latency or jitter requirements; such cases require architectural approval and clear justification.

### SAD-D-2: Use common module template for Sensors and Controls
Separate low-level device handling (Driver) from project integration (Utility) to maximise reuse and testability.

**Rules:**
- Each module should consist of two parts:
  - Driver: Code that directly interacts with hardware, samples inputs, performs filtering and basic signal processing. Drivers MUST be OS-agnostic (no RTOS APIs).
  - Utility: Code that integrates the driver with this project: reads configuration from the Database, implements task/thread behaviour, handles scheduling, and calls Driver APIs. Utilities MAY use FreeRTOS and other OS services.
- Each component resides in its own folder.
- Each component MUST provide at least:
  - drv\<Name\>.h
  - drv\<Name\>.c
  - ul\<Name\>.h
  - ul\<Name\>.c

where \<Name> is the component short name (e.g. drvImu.c / ulImu.c)

**Exceptions:**
- Any exception (e.g. extreme real-time constraints) requires architectural justification and explicit approval.

### SAD-D-3: The states of driver must be determinated
Each driver MUST be implemented as a state machine to handle its operational states in a deterministic manner. The switch-case (switch-programming) state machine approach SHOULD be used for clarity and maintainability.

**Rules:**
- Drivers MUST define all possible operational states explicitly (e.g. INIT, READY, ERROR, RUNNING, STOPPED).
- State transitions MUST be handled using a switch-case structure or equivalent, with clear entry and exit conditions for each state.
- Drivers MUST NOT perform actions outside their defined state transitions.
- State variables MUST be private to the driver and not exposed to other modules.
- Each driver MUST have a public function `run()` that should be called periodically and used as the clock for the state machine.
- If a driver requires a delay in its logic, it MUST be implemented as a separate state, and the driver MUST measure the time elapsed since entering that state.

**Exceptions:**
- Exceptions to the state machine approach (e.g. for extremely simple drivers or hardware with stateless operation) require architectural approval and explicit justification.

## Database
This is the central configuration and state store used by all components to read settings and publish state/telemetry.

The Database component provides a lightweight, type-safe parameter management system with persistent storage capabilities. It uses a metadata-driven approach where parameters are stored sequentially in a byte array, with their layout defined by a metadata table.

### Architecture

The Database component consists of two main parts:

1. **ulDatabase**: Manages parameters in RAM, provides type-safe API for reading and writing parameters using unique IDs.
2. **ulStorage**: Handles serialization and persistence of parameters marked as persistent to non-volatile memory (e.g., Flash, EEPROM).

### Design Principles

- **Metadata-driven layout**: Parameter positions in the data array are computed from metadata (offset and type), allowing flexible schema definition.
- **Type safety**: Separate getter/setter functions per data type prevent type mismatches.
- **Selective persistence**: Only parameters marked with the `persistent` flag are saved to non-volatile storage.
- **Compact storage**: Parameters are stored sequentially in a byte array to minimize RAM footprint.
- **Persistent parameters ordering**: All persistent parameters MUST be placed at the beginning of the metadata table. This allows efficient storage operations by treating persistent data as a contiguous block.
- **Default values**: Each parameter has a default value stored in metadata as a float, which is type-cast during initialization or reset operations.

### Class Diagram

```plantuml
    @startuml
    enum ParamType {
        UINT8
        INT8
        UINT16
        INT16
        UINT32
        INT32
        FLOAT
    }

    enum ParamId {
        PARAM_MOTOR_P_GAIN
        PARAM_HEADING_OFFSET
        PARAM_BATTERY_LEVEL
        PARAM_ENCODER_COUNT
        PARAM_COUNT
    }

    class ParamMetadata {
        - uint16_t offset
        - ParamType type
        - bool persistent
        - float defaultValue
        + getSize() : uint8_t
    }

    class ulDatabase {
        - uint8_t* dataArray
        - ParamMetadata* metadataTable
        - uint16_t metadataCount
        - uint16_t dataArraySize
        + init(metadataTable, metadataCount) : bool
        + setUint8(id, value) : bool
        + getUint8(id, *value) : bool
        + setInt8(id, value) : bool
        + getInt8(id, *value) : bool
        + setUint16(id, value) : bool
        + getUint16(id, *value) : bool
        + setInt16(id, value) : bool
        + getInt16(id, *value) : bool
        + setUint32(id, value) : bool
        + getUint32(id, *value) : bool
        + setInt32(id, value) : bool
        + getInt32(id, *value) : bool
        + setFloat(id, value) : bool
        + getFloat(id, *value) : bool
        - getMetadata(id) : ParamMetadata*
        - validateId(id) : bool
    }

    class ulStorage {
        - ulDatabase* database
        + init(database) : bool
        + save() : bool
        + load() : bool
        + erase() : bool
        + factoryReset() : bool
        - writePersistentParams() : bool
        - readPersistentParams() : bool
        - calculateChecksum() : uint32_t
        - verifyChecksum() : bool
    }

    ulDatabase "1" *-- "many" ParamMetadata : contains
    ulDatabase "1" o-- "1" ParamType : uses
    ulDatabase "1" o-- "1" ParamId : uses
    ulStorage "1" --> "1" ulDatabase : accesses
    @enduml
```

### API Description

#### ulDatabase

**Public Methods:**
- `init(metadataTable, metadataCount)`: Initializes the database with a metadata table defining all parameters. Calculates required data array size and allocates memory. Sets all parameters to their default values.
- `setUint8/Int8/Uint16/Int16/Uint32/Int32/Float(id, value)`: Type-specific setters. Each validates the ID, checks type compatibility, and writes the value at the computed offset.
- `getUint8/Int8/Uint16/Int16/Uint32/Int32/Float(id, *value)`: Type-specific getters. Each validates the ID, checks type compatibility, and reads the value from the computed offset.
- `reset(id)`: Resets a parameter to its default value from metadata.
- `resetAll()`: Resets all parameters to their default values.


#### ulStorage

**Public Methods:**
- `init(database)`: Initializes the storage module with a reference to the ulDatabase instance.
- `save()`: Writes all parameters marked as `persistent` to non-volatile memory along with a checksum.
- `load()`: Reads persistent parameters from non-volatile memory, verifies checksum, and updates the ulDatabase.
- `erase()`: Erases persistent storage (factory reset).
- `factoryReset()`: Erases persistent storage and resets all parameters to default values.

**Private Methods:**
- `writePersistentParams()`: Internal method to serialize and write persistent parameters.
- `readPersistentParams()`: Internal method to read and deserialize persistent parameters.
- `calculateChecksum()`: Computes CRC32 or similar checksum over persistent data.
- `verifyChecksum()`: Validates data integrity after reading from storage.

### Usage Example

```c
// Define parameter IDs as enum
typedef enum {
    // Persistent parameters MUST be first
    PARAM_MOTOR_P_GAIN = 0,
    PARAM_HEADING_OFFSET,
    // Non-persistent parameters after
    PARAM_BATTERY_LEVEL,
    PARAM_ENCODER_COUNT,
    PARAM_COUNT  // Total number of parameters
} ParamId;

// Define metadata table (persistent parameters at the beginning)
ParamMetadata metadata[] = {
    {.offset = 0,  .type = FLOAT,  .persistent = true,  .defaultValue = 1.5f},  // PARAM_MOTOR_P_GAIN
    {.offset = 4,  .type = INT16,  .persistent = true,  .defaultValue = 0.0f},  // PARAM_HEADING_OFFSET
    {.offset = 6,  .type = UINT8,  .persistent = false, .defaultValue = 0.0f},  // PARAM_BATTERY_LEVEL
    {.offset = 7,  .type = UINT32, .persistent = false, .defaultValue = 0.0f},  // PARAM_ENCODER_COUNT
};

// Initialize (sets all parameters to default values)
ulDatabase_init(metadata, PARAM_COUNT);
ulStorage_init(&database);

// Load persisted values (overrides defaults for persistent params)
ulStorage_load();

// Read/Write parameters using enum
uint8_t battLevel;
ulDatabase_getUint8(PARAM_BATTERY_LEVEL, &battLevel);

float pGain = 1.5f;
ulDatabase_setFloat(PARAM_MOTOR_P_GAIN, pGain);

// Reset single parameter to default
ulDatabase_reset(PARAM_MOTOR_P_GAIN);

// Factory reset: erase storage and reset all to defaults
ulStorage_factoryReset();

// Save persistent parameters
ulStorage_save();
```

### List of parameters

Below is a table of Database parameters used by the MotorsController:

| Name                        | Type   | Description                                 | Persistent | Default Value |
|-----------------------------|--------|---------------------------------------------|------------|--------------|
| MOTOR_FL_KP                 | float  | PID Kp coefficient for front-left motor     | yes        | (project default) |
| MOTOR_FL_KI                 | float  | PID Ki coefficient for front-left motor     | yes        | (project default) |
| MOTOR_FL_KD                 | float  | PID Kd coefficient for front-left motor     | yes        | (project default) |
| MOTOR_FR_KP                 | float  | PID Kp coefficient for front-right motor    | yes        | (project default) |
| MOTOR_FR_KI                 | float  | PID Ki coefficient for front-right motor    | yes        | (project default) |
| MOTOR_FR_KD                 | float  | PID Kd coefficient for front-right motor    | yes        | (project default) |
| MOTOR_RL_KP                 | float  | PID Kp coefficient for rear-left motor      | yes        | (project default) |
| MOTOR_RL_KI                 | float  | PID Ki coefficient for rear-left motor      | yes        | (project default) |
| MOTOR_RL_KD                 | float  | PID Kd coefficient for rear-left motor      | yes        | (project default) |
| MOTOR_RR_KP                 | float  | PID Kp coefficient for rear-right motor     | yes        | (project default) |
| MOTOR_RR_KI                 | float  | PID Ki coefficient for rear-right motor     | yes        | (project default) |
| MOTOR_RR_KD                 | float  | PID Kd coefficient for rear-right motor     | yes        | (project default) |
| MOTOR_FL_SETPOINT           | int32  | Target RPM for front-left motor             | no         | 0            |
| MOTOR_FR_SETPOINT           | int32  | Target RPM for front-right motor            | no         | 0            |
| MOTOR_RL_SETPOINT           | int32  | Target RPM for rear-left motor              | no         | 0            |
| MOTOR_RR_SETPOINT           | int32  | Target RPM for rear-right motor             | no         | 0            |
| MOTOR_FL_RPM                | int32  | Measured RPM for front-left motor           | no         | 0            |
| MOTOR_FR_RPM                | int32  | Measured RPM for front-right motor          | no         | 0            |
| MOTOR_RL_RPM                | int32  | Measured RPM for rear-left motor            | no         | 0            |
| MOTOR_RR_RPM                | int32  | Measured RPM for rear-right motor           | no         | 0            |
| MOTORS_CONTROL_PERIOD_MS    | uint16 | Control loop period in milliseconds         | yes        | 10           |

## Communication Hub

The Communication Hub manages external communication interfaces, providing a unified interface for data exchange between the chassis controller and external systems.

### Architecture

The Communication Hub component consists of two main parts:

1. **drvComHub**: Low-level communication driver handling protocol implementation and hardware interfaces.
2. **ulComHub**: Utility layer integrating the driver with the Database and implementing communication tasks.

### Design Principles

- Protocol abstraction for UART, CAN, USB, etc.
- Message routing to/from Database.
- Error handling and communication health monitoring.

### Class Diagram

```plantuml
@startuml
' TODO: Add class diagram
@enduml
```

### API Description

#### drvComHub

**Public Methods:**
- TODO

**Private Methods:**
- TODO

#### ulComHub

**Public Methods:**
- TODO

**Private Methods:**
- TODO

### Usage Example

```c
// TODO: Add usage example
```

## MotorsController

The MotorsController is responsible for precise speed control of each motor using closed-loop PID regulation. It coordinates the reading of configuration and runtime parameters from the Database, executes the control loop, and commands the motor drivers accordingly.

### Architecture

The MotorsController component consists of four motor driver instances and a utility layer:

1. **drvMotors**: Low-level driver handling PWM generation and motor control for each motor.
2. **ulMotorsController**: Utility layer that manages control logic, interacts with the Database, and contains PID regulation for each motor as members.
3. **ulPID**: Each ulMotorsController instance contains four ulPID members for closed-loop speed control.

### Design Principles

- **Database-driven configuration**: On startup, `ulMotorsController` reads PID coefficients (Kp, Ki, Kd) for each motor from the Database and configures the corresponding `ulPID` member.
- **Periodic control loop**: `ulMotorsController` runs a periodic task (period is a parameter stored in the Database) that:
  - Reads the current setpoint (target RPM) and measured RPM for each motor from the Database.
  - Passes these values to the corresponding `ulPID` member, which computes the required PWM output.
  - Sends the computed PWM value to the appropriate `drvMotors` driver for actuation.
- **Parameterization**: The control loop period and all PID coefficients are configurable via the Database, allowing runtime tuning and adaptation.
- **Separation of concerns**: `ulMotorsController` handles high-level control logic and Database interaction, while each `drvMotors` is responsible for hardware actuation.

### Class Diagram

```plantuml
@startuml
class ulPID {
    -float kp
    -float ki
    -float kd
    -float integral
    -float prevError
    +init(kp, ki, kd)
    +reset()
    +compute(setpointRPM, currentRPM) : float
    ...
}

class drvMotor {
    +init()
    +disable()
    +enable()
    +setPwm(pwmValue)
    ...
}

class ulMotorsController {
    -drvMotor motors[4]
    -ulPID pids[4]
    -periodMs
    +init()
    +run()
    ...
}

ulMotorsController *-- "drvMotor" : 4
ulMotorsController *-- "ulPID" : 4

@enduml
```

### API Description

#### ulMotorsController
- `init()`: Initialize all motors and PID controllers, load configuration from Database.
- `run()`: Execute the periodic control loop (read setpoints and feedback, compute PID, update motors).

#### drvMotors
- `init()`: Initialize the motor driver hardware.
- `enable()`: Enable the motor output. After calling `enable()`, `setPwm()` commands are accepted.
- `disable()`: Disable the motor output (safe state). After calling `disable()`, PWM is forced to 0 and further `setPwm()` calls have no effect until `enable()` is called again.
- `setPwm(pwmValue)`: Set the PWM value for the motor (only works if enabled).

#### ulPID
- `init(kp, ki, kd)`: Initialize the PID controller with given coefficients.
- `reset()`: Reset the PID internal state (integral, previous error).
- `compute(setpoint, measured)`: Calculate the control output based on setpoint and measured value.
- `setParams(kp, ki, kd)`: Update PID coefficients.

### Dynamic Behavior

#### Initialization

```plantuml
@startuml
actor System
participant ulMotorsController
participant drvMotor as "drvMotor[4]"
participant ulPID as "ulPID[4]"
participant Database

System -> ulMotorsController: init()
loop for each motor
    ulMotorsController -> Database: getFloat(MOTOR_XX_KP, &kp)
    ulMotorsController -> Database: getFloat(MOTOR_XX_KI, &ki)
    ulMotorsController -> Database: getFloat(MOTOR_XX_KD, &kd)
    ulMotorsController -> ulPID: init(kp, ki, kd)
    ulMotorsController -> drvMotor: init()
    ulMotorsController -> drvMotor: enable()
end
ulMotorsController -> Database: getUint16(MOTORS_CONTROL_PERIOD_MS, &periodMs)
@enduml
```

#### Control Loop

```plantuml
@startuml
actor RTOS
participant ulMotorsController
participant drvMotor as "drvMotor[4]"
participant ulPID as "ulPID[4]"
participant Database

RTOS -> ulMotorsController: run()
loop for each motor
    ulMotorsController -> Database: getInt32(MOTOR_XX_SETPOINT, &setpoint)
    ulMotorsController -> Database: getInt32(MOTOR_XX_RPM, &rpm)
    ulMotorsController -> ulPID: compute(setpoint, rpm)
    ulPID -> ulMotorsController: pwm value
    ulMotorsController -> drvMotor: setPwm(pwmValue)
end
@enduml
```

#### Disable / Enable

```plantuml
@startuml
participant ulMotorsController
participant drvMotor as "drvMotor[4]"

ulMotorsController -> drvMotor: disable()
drvMotor -> drvMotor: setPwm(0)
...
ulMotorsController -> drvMotor: setPwm()
note right of drvMotor
  setPwm() will be ignored until enable() is called
end note
...
ulMotorsController -> drvMotor: enable()
...
ulMotorsController -> drvMotor: setPwm()
note right of drvMotor
  setPwm() accepted
end note
@enduml
```

## ADC Module

The ADC Module samples analog channels (battery voltage, current sensors, etc.) and provides filtered measurement data to the Database.

### Architecture

The ADC Module component consists of two main parts:

1. **drvAdc**: Low-level ADC driver handling hardware configuration and raw sample acquisition.
2. **ulAdc**: Utility layer implementing filtering, calibration, and Database updates.

### Design Principles

- Multi-channel sampling.
- Signal filtering and calibration.

### Class Diagram

```plantuml
@startuml
' TODO: Add class diagram
@enduml
```

### API Description

#### drvAdc

**Public Methods:**
- TODO

**Private Methods:**
- TODO

#### ulAdc

**Public Methods:**
- TODO

**Private Methods:**
- TODO

### Usage Example

```c
// TODO: Add usage example
```

## IMU Module

The IMU Module provides accelerometer and gyroscope data for motion sensing and orientation estimation.

### Architecture

The IMU Module component consists of two main parts:

1. **drvImu**: Low-level IMU driver handling I2C/SPI communication and sensor configuration.
2. **ulImu**: Utility layer implementing data fusion, calibration, and Database updates.

### Design Principles

- Sensor fusion and calibration.
- High-rate sampling.

### Class Diagram

```plantuml
@startuml
' TODO: Add class diagram
@enduml
```

### API Description

#### drvImu

**Public Methods:**
- TODO

**Private Methods:**
- TODO

#### ulImu

**Public Methods:**
- TODO

**Private Methods:**
- TODO

### Usage Example

```c
// TODO: Add usage example
```

## Compass Module

The Compass Module provides heading information using magnetometer data for navigation and orientation.

### Architecture

The Compass Module component consists of two main parts:

1. **drvCompass**: Low-level magnetometer driver handling I2C communication and sensor configuration.
2. **ulCompass**: Utility layer implementing heading calculation, calibration, and Database updates.

### Design Principles

- Magnetic calibration and tilt compensation.

### Class Diagram

```plantuml
@startuml
' TODO: Add class diagram
@enduml
```

### API Description

#### drvCompass

**Public Methods:**
- TODO

**Private Methods:**
- TODO

#### ulCompass

**Public Methods:**
- TODO

**Private Methods:**
- TODO

### Usage Example

```c
// TODO: Add usage example
```

## GPS Module

The GPS Module provides position, velocity, and time information for navigation and localization.

### Architecture

The GPS Module component consists of two main parts:

1. **drvGps**: Low-level GPS driver handling UART communication and protocol parsing.
2. **ulGps**: Utility layer implementing data validation, conversion, and Database updates.

### Design Principles

- Protocol support and fix quality monitoring.

### Class Diagram

```plantuml
@startuml
' TODO: Add class diagram
@enduml
```

### API Description

#### drvGps

**Public Methods:**
- TODO

**Private Methods:**
- TODO

#### ulGps

**Public Methods:**
- TODO

**Private Methods:**
- TODO

### Usage Example

```c
// TODO: Add usage example
```

## Encoders Module

The Encoders Module reads wheel encoder counts and calculates wheel speed for odometry and motor control.

### Architecture

The Encoders Module component consists of two main parts:

1. **drvEncoders**: Low-level encoder driver handling timer/capture hardware and pulse counting.
2. **ulEncoders**: Utility layer implementing speed calculation, direction detection, and Database updates.

### Design Principles

- High-resolution counting and speed calculation.

### Class Diagram

```plantuml
@startuml
' TODO: Add class diagram
@enduml
```

### API Description

#### drvEncoders

**Public Methods:**
- TODO

**Private Methods:**
- TODO

#### ulEncoders

**Public Methods:**
- TODO

**Private Methods:**
- TODO

### Usage Example

```c
// TODO: Add usage example
```

## LEDsController

The LEDsController manages visual indicators (LEDs) for system status, alerts, and user feedback.

### Architecture

The LEDsController component consists of two main parts:

1. **drvLeds**: Low-level LED driver handling GPIO control and PWM for brightness.
2. **ulLeds**: Utility layer implementing patterns, animations, and Database-driven control.

### Design Principles

- Pattern library and priority management.

### Class Diagram

```plantuml
@startuml
' TODO: Add class diagram
@enduml
```

### API Description

#### drvLeds

**Public Methods:**
- TODO

**Private Methods:**
- TODO

#### ulLeds

**Public Methods:**
- TODO

**Private Methods:**
- TODO

### Usage Example

```c
// TODO: Add usage example
```

## BuzzerController

The BuzzerController manages audible alerts for warnings, errors, and user feedback.

### Architecture

The BuzzerController component consists of two main parts:

1. **drvBuzzer**: Low-level buzzer driver handling PWM frequency generation.
2. **ulBuzzer**: Utility layer implementing tones, melodies, and Database-driven control.

### Design Principles

- Tone generation and melody sequences.

### Class Diagram

```plantuml
@startuml
' TODO: Add class diagram
@enduml
```

### API Description

#### drvBuzzer

**Public Methods:**
- TODO

**Private Methods:**
- TODO

#### ulBuzzer

**Public Methods:**
- TODO

**Private Methods:**
- TODO

### Usage Example

```c
// TODO: Add usage example
```

## Logger

The Logger provides centralized logging functionality for all components, supporting multiple output channels and log levels.

### Architecture

The Logger is implemented as a standalone utility component:

1. **ulLogger**: Logging utility providing formatted output to multiple destinations (UART, file, etc.).

### Design Principles

- Multi-channel output and severity levels.

### Class Diagram

```plantuml
@startuml
' TODO: Add class diagram
@enduml
```

### API Description

#### ulLogger

**Public Methods:**
- TODO

**Private Methods:**
- TODO

### Usage Example

```c
// TODO: Add usage example
```

