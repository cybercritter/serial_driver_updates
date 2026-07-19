#ifndef SERIAL_DRIVER_H
#define SERIAL_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

#include "errors.h"

#ifdef __cplusplus
extern "C"
{
#endif

  Error portInitialization (uint8_t port);
  Error disableAllInterrupts (uint8_t port);
  Error setDiscreteState (uint8_t port, bool state);
  Error getDiscreteState (uint8_t port, bool *state);
  Error setLoopbackState (uint8_t port, bool state);
  Error setFifoState (uint8_t port, bool state);
  Error getFifoState (uint8_t port, bool *state);

#ifdef SERIAL_DRIVER_TESTING
  void serialDriverSetRegisterBaseForTest (uintptr_t base_addr);
#endif

#ifdef __cplusplus
}
#endif

#endif /* SERIAL_DRIVER_H */
