#include "Common.h"
#include "MicoDefaults.h"

#ifdef MICO_DEFAULT_APPLICATION_STACK_SIZE
uint32_t  app_stack_size = MICO_DEFAULT_APPLICATION_STACK_SIZE; 
#else
uint32_t  app_stack_size = 1500;
#endif
