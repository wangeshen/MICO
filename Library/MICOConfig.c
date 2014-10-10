#include "Common.h"
#include "MicoDefaults.h"

#ifdef MICO_DEFAULT_APPLICATION_STACK_SIZE
uint32_t  app_stack_size = MICO_DEFAULT_APPLICATION_STACK_SIZE; 
#else
uint32_t  app_stack_size = 1500;
#endif

const uint32_t  mico_cpu_clock_hz = 120000000; // CPU CLock is 120MHz
const uint32_t  mico_tick_rate_hz = 1000; // OS tick is 1000Hz

