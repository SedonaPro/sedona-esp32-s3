//
// bclimateEsp32::Esp32PlatformService natives.
//
#include "sedona.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"

// static void rebootSystem()
Cell bclimateEsp32_Esp32PlatformService_rebootSystem(SedonaVM* vm, Cell* params)
{
  printf("bclimateEsp32: reboot requested\n");
  fflush(stdout);
  esp_restart();
  return nullCell;
}

// static int heapMinFree()
Cell bclimateEsp32_Esp32PlatformService_heapMinFree(SedonaVM* vm, Cell* params)
{
  Cell result;
  result.ival = (int32_t)heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);
  return result;
}

// static int uptimeSec()
Cell bclimateEsp32_Esp32PlatformService_uptimeSec(SedonaVM* vm, Cell* params)
{
  Cell result;
  result.ival = (int32_t)(esp_timer_get_time() / 1000000LL);
  return result;
}

// static int chipResetReason()
Cell bclimateEsp32_Esp32PlatformService_chipResetReason(SedonaVM* vm, Cell* params)
{
  Cell result;
  result.ival = (int32_t)esp_reset_reason();
  return result;
}
