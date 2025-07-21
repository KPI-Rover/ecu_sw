#include "ul_ulog.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os2.h"
#include "usbd_cdc_if.h"
#include "usbd_def.h"
#include "main.h"
#include "portable.h"
#include <string.h>
#include <stdio.h>
#include <string.h>

#define LOG_QUEUE_LENGTH    10
#define MAX_LOG_MESSAGE_SIZE 256

static void ulogTask(void *argument);

static osThreadId_t ulogTaskHandle;
static StaticQueue_t xUlogQueueBuffer;
static uint8_t ucUlogQueueStorage[LOG_QUEUE_LENGTH * MAX_LOG_MESSAGE_SIZE];
static QueueHandle_t xULogQueue = NULL;

static const osThreadAttr_t ulogTask_attributes = {
  .name = "ulogTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};


void ul_ulog_init()
{ 

  xULogQueue = xQueueCreateStatic(LOG_QUEUE_LENGTH, MAX_LOG_MESSAGE_SIZE,
                               ucUlogQueueStorage, &xUlogQueueBuffer);
  if (xULogQueue == NULL)
  {
    Error_Handler();
  }

  ulogTaskHandle = osThreadNew(ulogTask, NULL, &ulogTask_attributes);
  if  (ulogTaskHandle == NULL)
  {
    Error_Handler();
  }
}

void ul_ulog_send(ulog_level_t level, const char *filename, char *msg)
{
  if (xULogQueue == NULL || msg == NULL)
  {
    return;
  }

  char logBuffer[MAX_LOG_MESSAGE_SIZE];
  int len;
  size_t msg_len = strlen(msg);

  // Extract just the filename from the full path
  const char *fbasename = strrchr(filename, '/');
  if (fbasename) {
    fbasename++; // Skip the '/'
  } else {
    fbasename = filename; // No path separator found
  }

  // Truncate message if too long, leaving space for timestamp, level, and filename
  if (msg_len > (MAX_LOG_MESSAGE_SIZE - 60))
  {
    msg_len = MAX_LOG_MESSAGE_SIZE - 60;
  }

  TickType_t currentTime = xTaskGetTickCount();
  len = snprintf(logBuffer, sizeof(logBuffer), "[%lu][%s][%s] %.*s\r\n",
                 (unsigned long)currentTime,
                 ulog_level_name(level),
                 fbasename,
                 (int)msg_len, msg);

  // Ensure buffer is always null-terminated
  if (len < 0)
  {
    // snprintf failed
    return;
  }
  if (len >= sizeof(logBuffer))
  {
    logBuffer[sizeof(logBuffer) - 1] = '\0';
  }
  
   BaseType_t xHigherPriorityTaskWoken = pdFALSE;
   if (xPortIsInsideInterrupt()) {
       xQueueSendFromISR(xULogQueue, logBuffer, &xHigherPriorityTaskWoken);
       portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
   } else {
       xQueueSend(xULogQueue, logBuffer, pdMS_TO_TICKS(10));
   }
}

static void ulogTask(void *argument)
{
  // TODO: Try to get rid of this delay
  // Wait a bit for USB to initialize before starting logging
  osDelay(2000);

  ULOG_INIT();
  ULOG_SUBSCRIBE(ul_ulog_send, ULOG_DEBUG_LEVEL);

  for (;;)
  {
    char logMessage[MAX_LOG_MESSAGE_SIZE];
    BaseType_t result = xQueueReceive(xULogQueue, logMessage, portMAX_DELAY);

    if (result == pdTRUE)
    {
      logMessage[MAX_LOG_MESSAGE_SIZE - 1] = '\0';
      CDC_Transmit_FS((uint8_t *)logMessage, strlen(logMessage));
    }
  }
}
