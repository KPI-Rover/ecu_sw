# uLog Logger

A lightweight logging system for the KPI Rover ECU software that provides structured logging with multiple log levels and configurable output destinations. Integrated with FreeRTOS for thread-safe operation across multiple tasks.

## Usage Example

Once initialized, you can use logging macros throughout your application:

```c
#include "ulog.h"

void StartDefaultTask(void *argument)
{
    uint32_t count = 0;
    
    ULOG_INFO("Default task started");
    
    for(;;)
    {
        ULOG_DEBUG("Hello from default task %d", count++);
        vTaskDelay(pdMS_TO_TICKS(101)); // Wait for ~100ms
    }
}

```

## Available Log Levels

The logger supports multiple log levels in order of increasing severity:

| Level | Macro | Numeric Value | Description |
|-------|-------|---------------|-------------|
| **TRACE** | `ULOG_TRACE()` | 100 | Very detailed information for tracing program flow |
| **DEBUG** | `ULOG_DEBUG()` | 101 | Detailed information for diagnosing problems |
| **INFO** | `ULOG_INFO()` | 102 | General information about program execution |
| **WARNING** | `ULOG_WARNING()` | 103 | Something unexpected happened, but software still works |
| **ERROR** | `ULOG_ERROR()` | 104 | Serious problem occurred, some function failed |
| **CRITICAL** | `ULOG_CRITICAL()` | 105 | Very serious error, program may be unable to continue |
| **ALWAYS** | `ULOG_ALWAYS()` | 106 | Messages that should always be logged |

### Example Output

When properly configured, the logger produces formatted output like this:

```
[20301][DEBUG][main.c] Hello from default task 201
[21302][INFO][sensor_task.c] Temperature: 23.5°C
[22303][WARNING][sensor_task.c] High temperature detected: 81.2°C
[23304][ERROR][can_driver.c] CAN transmission failed, error code: 0x42
```
