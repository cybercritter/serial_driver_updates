#include "serial_driver.h"
#include "x17v358_regs.h"

#include <stddef.h>

enum
{
  X17V358_MAX_PORT = 12u
};

static uintptr_t g_reg_base_addr = (uintptr_t)UINT64_C (0xFC0000000);

static inline bool
isValidPort (uint8_t port)
{
  return port <= X17V358_MAX_PORT;
}

static inline bool
isRegisterBaseValid (void)
{
  return g_reg_base_addr != 0u;
}

static volatile x17v358_channel_regs_t *
getChannelRegs (uint8_t port)
{
  const uintptr_t channel_addr = g_reg_base_addr + ((uintptr_t)port * 0x400U);
  return (volatile x17v358_channel_regs_t *)channel_addr;
}

#ifdef SERIAL_DRIVER_TESTING
void
serialDriverSetRegisterBaseForTest (uintptr_t base_addr)
{
  g_reg_base_addr = base_addr;
}
#endif

Error
portInitialization (uint8_t port)
{
  if (!isValidPort (port))
    {
      return ERROR_INVALID_PORT;
    }

  if (!isRegisterBaseValid ())
    {
      return ERROR_NULL_REGISTER_BASE;
    }

  /* Initialize all sub-functions - collect error from any failures */
  Error err = disableAllInterrupts (port);
  if (err != ERROR_SUCCESS)
    {
      return err;
    }

  err = setLoopbackState (port, false);
  if (err != ERROR_SUCCESS)
    {
      return err;
    }

  err = setFifoState (port, true);
  if (err != ERROR_SUCCESS)
    {
      return err;
    }

  err = setDiscreteState (port, false);
  if (err != ERROR_SUCCESS)
    {
      return err;
    }

  return ERROR_SUCCESS;
}

Error
getFifoState (uint8_t port, bool *state)
{
  x17v358_fcr_isr_reg_t fcr = { 0 };
  volatile x17v358_channel_regs_t *regs = NULL;

  if (!isValidPort (port))
    {
      return ERROR_INVALID_PORT;
    }

  if (state == NULL)
    {
      return ERROR_NULL_PTR;
    }

  if (!isRegisterBaseValid ())
    {
      return ERROR_NULL_REGISTER_BASE;
    }

  regs = getChannelRegs (port);
  if (regs == NULL)
    {
      return ERROR_NULL_PTR;
    }

  fcr.raw = regs->isr_fcr.raw;
  *state = fcr.fcr_bits.fifo_enable ? true : false;
  return ERROR_SUCCESS;
}

Error
disableAllInterrupts (uint8_t port)
{
  x17v358_ier_reg_t ier = { 0 };
  volatile x17v358_channel_regs_t *regs = NULL;

  if (!isValidPort (port))
    {
      return ERROR_INVALID_PORT;
    }

  if (!isRegisterBaseValid ())
    {
      return ERROR_NULL_REGISTER_BASE;
    }

  regs = getChannelRegs (port);
  if (regs == NULL)
    {
      return ERROR_NULL_PTR;
    }

  ier.raw = regs->ier_dlh.raw;
  ier.bits.rx_data_available_int_en = 0u;
  ier.bits.tx_holding_register_empty_int_en = 0u;
  ier.bits.rx_line_status_int_en = 0u;
  ier.bits.modem_status_int_en = 0u;
  ier.bits.sleep_mode_int_en = 0u;
  ier.bits.xoff_int_en = 0u;
  ier.bits.rts_cts_int_en = 0u;
  ier.bits.cts_rts_int_en = 0u;

  regs->ier_dlh.raw = ier.raw;

  return ERROR_SUCCESS;
}

Error
setDiscreteState (uint8_t port, bool enable)
{
  x17v358_mcr_reg_t mcr = { 0 };
  volatile x17v358_channel_regs_t *regs = NULL;

  if (!isValidPort (port))
    {
      return ERROR_INVALID_PORT;
    }

  if (!isRegisterBaseValid ())
    {
      return ERROR_NULL_REGISTER_BASE;
    }

  regs = getChannelRegs (port);
  if (regs == NULL)
    {
      return ERROR_NULL_PTR;
    }

  mcr.raw = regs->mcr.raw;
  mcr.bits.rts = enable ? 1u : 0u;
  regs->mcr.raw = mcr.raw;

  return ERROR_SUCCESS;
}

Error
getDiscreteState (uint8_t port, bool *state)
{
  x17v358_mcr_reg_t mcr = { 0 };
  volatile x17v358_channel_regs_t *regs = NULL;

  if (!isValidPort (port))
    {
      return ERROR_INVALID_PORT;
    }

  if (state == NULL)
    {
      return ERROR_NULL_PTR;
    }

  if (!isRegisterBaseValid ())
    {
      return ERROR_NULL_REGISTER_BASE;
    }

  regs = getChannelRegs (port);
  if (regs == NULL)
    {
      return ERROR_NULL_PTR;
    }

  mcr.raw = regs->mcr.raw;
  *state = mcr.bits.rts ? true : false;

  return ERROR_SUCCESS;
}

Error
setLoopbackState (uint8_t port, bool enable)
{
  x17v358_mcr_reg_t mcr = { 0 };
  volatile x17v358_channel_regs_t *regs = NULL;

  if (!isValidPort (port))
    {
      return ERROR_INVALID_PORT;
    }

  if (!isRegisterBaseValid ())
    {
      return ERROR_NULL_REGISTER_BASE;
    }

  regs = getChannelRegs (port);
  if (regs == NULL)
    {
      return ERROR_NULL_PTR;
    }

  mcr.raw = regs->mcr.raw;
  mcr.bits.loopback = enable ? 1u : 0u;
  regs->mcr.raw = mcr.raw;

  return ERROR_SUCCESS;
}

Error
setFifoState (uint8_t port, bool enable)
{
  x17v358_fcr_isr_reg_t fcr = { 0 };
  volatile x17v358_channel_regs_t *regs = NULL;

  if (!isValidPort (port))
    {
      return ERROR_INVALID_PORT;
    }

  if (!isRegisterBaseValid ())
    {
      return ERROR_NULL_REGISTER_BASE;
    }

  regs = getChannelRegs (port);
  if (regs == NULL)
    {
      return ERROR_NULL_PTR;
    }

  fcr.raw = regs->isr_fcr.raw;
  fcr.fcr_bits.fifo_enable = enable ? 1u : 0u;
  regs->isr_fcr.raw = fcr.raw;

  return ERROR_SUCCESS;
}

