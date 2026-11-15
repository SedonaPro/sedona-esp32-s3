#pragma once
#include "FreeRTOS.h"
#include <sched.h>
#define vTaskDelay(t) esp32simTaskDelayMs((uint32_t)(t) * portTICK_PERIOD_MS)
#define taskYIELD()   sched_yield()
