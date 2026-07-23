#include "serial_driver.h"

#include <stddef.h>

#include "x17v358_regs.h"

enum { X17V358_MAX_PORT = 12u };

#define X17V358_DEFAULT_INPUT_CLOCK_HZ UINT32_C(125000000)
#define X17V358_DEFAULT_BAUD_RATE UINT32_C(115200)
#define X17V358_BAUD_FRACTION_STEPS UINT32_C(16)
#define X17V358_BAUD_DLD_MASK UINT8_C(0x0F)
#define X17V358_BAUD_MIN_DIVISOR_UNITS X17V358_BAUD_FRACTION_STEPS
#define X17V358_BAUD_MAX_DIVISOR_UNITS \
  ((UINT32_C(0xFFFF) * X17V358_BAUD_FRACTION_STEPS) + \
   (X17V358_BAUD_FRACTION_STEPS - UINT32_C(1)))
#define X17V358_BAUD_ERROR_SCALE UINT32_C(1000000)

port_config_t ports[MAX_NUM_PORTS + 1u] = {0};
static uintptr_t g_reg_base_addr = (uintptr_t)UINT64_C(0xFC0000000);

static inline bool isValidPort(uint8_t port) {
  return port <= X17V358_MAX_PORT;
}

static inline bool isRegisterBaseValid(void) { return g_reg_base_addr != 0u; }

static inline uint32_t roundDivideU64(uint64_t numerator,
                                      uint64_t denominator) {
  return (uint32_t)((numerator + (denominator / UINT64_C(2))) / denominator);
}

static volatile x17v358_channel_regs_t* getChannelRegs(uint8_t port) {
  const uintptr_t channel_addr = g_reg_base_addr + ((uintptr_t)port * 0x400U);
  return (volatile x17v358_channel_regs_t*)channel_addr;
}

static Error configurePortBaudRate(uint8_t port) {
  const uint32_t requested_baud_rate =
      (ports[port].baud_rate != 0u) ? ports[port].baud_rate
                                    : X17V358_DEFAULT_BAUD_RATE;
  x17v358_baud_rate_config_t baud_config = {0};
  volatile x17v358_channel_regs_t* regs = getChannelRegs(port);
  const Error err = calculateBaudDivisor(X17V358_DEFAULT_INPUT_CLOCK_HZ,
                                         requested_baud_rate, false,
                                         &baud_config);
  x17v358_lcr_reg_t saved_lcr = {0};
  x17v358_lcr_reg_t active_lcr = {0};
  x17v358_dld_reg_t dld = {0};

  if (err != ERROR_SUCCESS) {
    return err;
  }

  saved_lcr.raw = regs->lcr.raw;
  active_lcr = saved_lcr;
  active_lcr.bits.dlab = 1u;
  regs->lcr.raw = active_lcr.raw;

  regs->rhr_thr_dll.raw = baud_config.dll;
  regs->ier_dlh.raw = baud_config.dlm;

  dld.raw = regs->isr_fcr.raw;
  dld.bits.fraction = baud_config.dld;
  regs->isr_fcr.raw = dld.raw;

  saved_lcr.bits.dlab = 0u;
  regs->lcr.raw = saved_lcr.raw;
  ports[port].baud_rate = baud_config.actual_baud_rate;

  return ERROR_SUCCESS;
}

#ifdef SERIAL_DRIVER_TESTING
void serialDriverSetRegisterBaseForTest(uintptr_t base_addr) {
  g_reg_base_addr = base_addr;
}
#endif

Error portInitialization(uint8_t port) {
  if (!isValidPort(port)) {
    return ERROR_INVALID_PORT;
  }

  if (!isRegisterBaseValid()) {
    return ERROR_NULL_REGISTER_BASE;
  }

  /* Initialize all sub-functions - collect error from any failures */
  Error err = configurePortBaudRate(port);
  if (err != ERROR_SUCCESS) {
    return err;
  }

  err = disableAllInterrupts(port);
  if (err != ERROR_SUCCESS) {
    return err;
  }

  err = setLoopbackState(port, false);
  if (err != ERROR_SUCCESS) {
    return err;
  }

  err = setFifoState(port, true);
  if (err != ERROR_SUCCESS) {
    return err;
  }

  err = setDiscreteState(port, false);
  if (err != ERROR_SUCCESS) {
    return err;
  }



  return ERROR_SUCCESS;
}

Error getFifoState(uint8_t port, bool* state) {
  x17v358_fcr_isr_reg_t fcr = {0};
  volatile x17v358_channel_regs_t* regs = NULL;

  if (!isValidPort(port)) {
    return ERROR_INVALID_PORT;
  }

  if (state == NULL) {
    return ERROR_NULL_PTR;
  }

  if (!isRegisterBaseValid()) {
    return ERROR_NULL_REGISTER_BASE;
  }

  regs = getChannelRegs(port);
  if (regs == NULL) {
    return ERROR_NULL_PTR;
  }

  fcr.raw = regs->isr_fcr.raw;
  *state = fcr.fcr_bits.fifo_enable ? true : false;
  return ERROR_SUCCESS;
}

Error disableAllInterrupts(uint8_t port) {
  x17v358_ier_reg_t ier = {0};
  volatile x17v358_channel_regs_t* regs = NULL;

  if (!isValidPort(port)) {
    return ERROR_INVALID_PORT;
  }

  if (!isRegisterBaseValid()) {
    return ERROR_NULL_REGISTER_BASE;
  }

  regs = getChannelRegs(port);
  if (regs == NULL) {
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

Error setDiscreteState(uint8_t port, bool enable) {
  x17v358_mcr_reg_t mcr = {0};
  volatile x17v358_channel_regs_t* regs = NULL;

  if (!isValidPort(port)) {
    return ERROR_INVALID_PORT;
  }

  if (!isRegisterBaseValid()) {
    return ERROR_NULL_REGISTER_BASE;
  }

  regs = getChannelRegs(port);
  if (regs == NULL) {
    return ERROR_NULL_PTR;
  }

  mcr.raw = regs->mcr.raw;
  mcr.bits.rts = enable ? 1u : 0u;
  regs->mcr.raw = mcr.raw;

  return ERROR_SUCCESS;
}

Error getDiscreteState(uint8_t port, bool* state) {
  x17v358_mcr_reg_t mcr = {0};
  volatile x17v358_channel_regs_t* regs = NULL;

  if (!isValidPort(port)) {
    return ERROR_INVALID_PORT;
  }

  if (state == NULL) {
    return ERROR_NULL_PTR;
  }

  if (!isRegisterBaseValid()) {
    return ERROR_NULL_REGISTER_BASE;
  }

  regs = getChannelRegs(port);
  if (regs == NULL) {
    return ERROR_NULL_PTR;
  }

  mcr.raw = regs->mcr.raw;
  *state = mcr.bits.rts ? true : false;

  return ERROR_SUCCESS;
}

Error setLoopbackState(uint8_t port, bool enable) {
  x17v358_mcr_reg_t mcr = {0};
  volatile x17v358_channel_regs_t* regs = NULL;

  if (!isValidPort(port)) {
    return ERROR_INVALID_PORT;
  }

  if (!isRegisterBaseValid()) {
    return ERROR_NULL_REGISTER_BASE;
  }

  regs = getChannelRegs(port);
  if (regs == NULL) {
    return ERROR_NULL_PTR;
  }

  mcr.raw = regs->mcr.raw;
  mcr.bits.loopback = enable ? 1u : 0u;
  regs->mcr.raw = mcr.raw;

  return ERROR_SUCCESS;
}

Error setFifoState(uint8_t port, bool enable) {
  x17v358_fcr_isr_reg_t fcr = {0};
  volatile x17v358_channel_regs_t* regs = NULL;

  if (!isValidPort(port)) {
    return ERROR_INVALID_PORT;
  }

  if (!isRegisterBaseValid()) {
    return ERROR_NULL_REGISTER_BASE;
  }

  regs = getChannelRegs(port);
  if (regs == NULL) {
    return ERROR_NULL_PTR;
  }

  fcr.raw = regs->isr_fcr.raw;
  fcr.fcr_bits.fifo_enable = enable ? 1u : 0u;
  regs->isr_fcr.raw = fcr.raw;

  return ERROR_SUCCESS;
}

Error calculateBaudRate(uint32_t input_clock_hz,
                        uint8_t dll,
                        uint8_t dlm,
                        uint8_t dld,
                        bool prescaler_divide_by_4,
                        uint32_t* baud_rate) {
  const uint32_t prescaler = prescaler_divide_by_4 ? UINT32_C(4) : UINT32_C(1);
  const uint32_t integer_divisor = ((uint32_t)dlm << 8) | (uint32_t)dll;
  const uint32_t divisor_units =
      (integer_divisor * X17V358_BAUD_FRACTION_STEPS) +
      ((uint32_t)dld & X17V358_BAUD_DLD_MASK);
  const uint64_t denominator = (uint64_t)prescaler * divisor_units;

  if (baud_rate == NULL) {
    return ERROR_NULL_PTR;
  }

  if ((input_clock_hz == 0u) ||
      (divisor_units < X17V358_BAUD_MIN_DIVISOR_UNITS)) {
    return ERROR_INVALID_PARAM;
  }

  *baud_rate = roundDivideU64(input_clock_hz, denominator);
  return ERROR_SUCCESS;
}

Error calculateBaudDivisor(uint32_t input_clock_hz,
                           uint32_t requested_baud_rate,
                           bool prescaler_divide_by_4,
                           x17v358_baud_rate_config_t* config) {
  const uint32_t prescaler = prescaler_divide_by_4 ? UINT32_C(4) : UINT32_C(1);
  const uint64_t denominator = (uint64_t)prescaler * requested_baud_rate;
  uint32_t divisor_units = 0u;
  uint32_t integer_divisor = 0u;
  uint32_t baud_delta = 0u;
  x17v358_baud_rate_config_t result = {0};
  Error err = ERROR_SUCCESS;

  if (config == NULL) {
    return ERROR_NULL_PTR;
  }

  if ((input_clock_hz == 0u) || (requested_baud_rate == 0u)) {
    return ERROR_INVALID_PARAM;
  }

  divisor_units = roundDivideU64(input_clock_hz, denominator);
  if ((divisor_units < X17V358_BAUD_MIN_DIVISOR_UNITS) ||
      (divisor_units > X17V358_BAUD_MAX_DIVISOR_UNITS)) {
    return ERROR_INVALID_PARAM;
  }

  integer_divisor = divisor_units / X17V358_BAUD_FRACTION_STEPS;
  result.dll = (uint8_t)(integer_divisor & UINT32_C(0xFF));
  result.dlm = (uint8_t)((integer_divisor >> 8) & UINT32_C(0xFF));
  result.dld = (uint8_t)(divisor_units & X17V358_BAUD_DLD_MASK);

  err = calculateBaudRate(input_clock_hz, result.dll, result.dlm, result.dld,
                          prescaler_divide_by_4, &result.actual_baud_rate);
  if (err != ERROR_SUCCESS) {
    return err;
  }

  baud_delta = (result.actual_baud_rate > requested_baud_rate)
                   ? (result.actual_baud_rate - requested_baud_rate)
                   : (requested_baud_rate - result.actual_baud_rate);
  result.error_ppm = roundDivideU64((uint64_t)baud_delta *
                                        X17V358_BAUD_ERROR_SCALE,
                                    requested_baud_rate);

  *config = result;
  return ERROR_SUCCESS;
}

enum Error setPortConfiguration(uint8_t port) {
  if (!isValidPort(port)) {
    return ERROR_INVALID_PORT;
  }

  if (!isRegisterBaseValid()) {
    return ERROR_NULL_REGISTER_BASE;
  }

  return configurePortBaudRate(port);
}
