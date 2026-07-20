/**
 * @file serial_driver.h
 * @brief Public API for the XR17V358 serial driver.
 *
 * This header defines the public interface for controlling MaxLinear XR17V358
 * 8-channel PCIe UART devices. The driver provides functions to initialize
 * ports and manage UART configuration including interrupt control, loopback
 * mode, FIFO settings, and modem control signals.
 *
 * @section usage Usage
 * Typical usage flow:
 * 1. Call portInitialization() to configure a port with default settings
 * 2. Use setFifoState(), setLoopbackState(), etc. to adjust configuration
 * 3. Use getDiscreteState() and getFifoState() to query current state
 *
 * @section error_handling Error Handling
 * All functions return an Error code. ERROR_SUCCESS indicates successful
 * completion. Other error codes indicate specific failure conditions:
 * - ERROR_INVALID_PORT: Port number is out of valid range
 * - ERROR_NULL_PTR: Output parameter pointer is NULL
 * - ERROR_NULL_REGISTER_BASE: Register base address not initialized
 *
 * @author Serial Driver Development Team
 * @date 2026-07-20
 */

#ifndef SERIAL_DRIVER_H
#define SERIAL_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

#include "errors.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup DriverInit Port Initialization
 * @brief Functions for initializing serial ports.
 * @{
 */

/**
 * @brief Initialize a serial port with default configuration.
 *
 * Performs full initialization of the specified UART port with recommended
 * default settings:
 * - Disables all interrupt sources
 * - Disables loopback mode
 * - Enables the transmit/receive FIFO with 128-byte capacity
 * - Clears the RTS (Request To Send) modem control signal
 *
 * @param[in] port Port number to initialize [0, MAX_NUM_PORTS].
 *
 * @retval ERROR_SUCCESS Port initialization successful.
 * @retval ERROR_INVALID_PORT Port number is out of valid range.
 * @retval ERROR_NULL_REGISTER_BASE Register base address not initialized.
 *
 * @note This function must be called before using a port for the first time.
 * @note Register base must be set via serialDriverSetRegisterBaseForTest()
 *       before calling this function (in testing mode).
 *
 * @see setFifoState, setLoopbackState, disableAllInterrupts
 */
Error portInitialization (uint8_t port);

/** @} */

/**
 * @defgroup DriverInterrupts Interrupt Control
 * @brief Functions for managing UART interrupt generation.
 * @{
 */

/**
 * @brief Disable all interrupt sources on a port.
 *
 * Clears all interrupt enable bits in the Interrupt Enable Register (IER),
 * preventing any interrupt generation on the specified port. This includes:
 * - RX data available interrupts
 * - TX holding register empty interrupts
 * - RX line status interrupts (errors)
 * - Modem status change interrupts
 * - Sleep mode interrupts
 * - Xoff character detected interrupts
 * - RTS/CTS flow control interrupts
 *
 * @param[in] port Port number [0, MAX_NUM_PORTS].
 *
 * @retval ERROR_SUCCESS All interrupt sources disabled.
 * @retval ERROR_INVALID_PORT Port number is out of valid range.
 * @retval ERROR_NULL_REGISTER_BASE Register base address not initialized.
 *
 * @see portInitialization
 */
Error disableAllInterrupts (uint8_t port);

/** @} */

/**
 * @defgroup DriverModemControl Modem Control
 * @brief Functions for managing modem control signals.
 * @{
 */

/**
 * @brief Set or clear the RTS (Request To Send) modem control signal.
 *
 * Controls the Request To Send output signal on the specified port. This
 * signal is typically used for hardware flow control between the host and
 * external devices.
 *
 * @param[in] port Port number [0, MAX_NUM_PORTS].
 * @param[in] state true to assert RTS (set high), false to deassert (set low).
 *
 * @retval ERROR_SUCCESS RTS state changed successfully.
 * @retval ERROR_INVALID_PORT Port number is out of valid range.
 * @retval ERROR_NULL_REGISTER_BASE Register base address not initialized.
 *
 * @see getDiscreteState
 */
Error setDiscreteState (uint8_t port, bool state);

/**
 * @brief Read the current RTS (Request To Send) modem control signal state.
 *
 * Queries the RTS output signal state on the specified port.
 *
 * @param[in] port Port number [0, MAX_NUM_PORTS].
 * @param[out] state Pointer to bool to receive current RTS state.
 *            true = RTS asserted (high), false = RTS deasserted (low).
 *
 * @retval ERROR_SUCCESS RTS state retrieved successfully.
 * @retval ERROR_INVALID_PORT Port number is out of valid range.
 * @retval ERROR_NULL_PTR state pointer is NULL.
 * @retval ERROR_NULL_REGISTER_BASE Register base address not initialized.
 *
 * @see setDiscreteState
 */
Error getDiscreteState (uint8_t port, bool *state);

/** @} */

/**
 * @defgroup DriverLoopback Loopback Mode
 * @brief Functions for managing serial loopback mode.
 * @{
 */

/**
 * @brief Enable or disable loopback mode on a port.
 *
 * Loopback mode connects the transmitter output directly to the receiver
 * input, allowing the port to test itself without external connections.
 * Useful for diagnostics and self-testing.
 *
 * @param[in] port Port number [0, MAX_NUM_PORTS].
 * @param[in] enable true to enable loopback mode, false to disable.
 *
 * @retval ERROR_SUCCESS Loopback mode state changed successfully.
 * @retval ERROR_INVALID_PORT Port number is out of valid range.
 * @retval ERROR_NULL_REGISTER_BASE Register base address not initialized.
 *
 * @note In loopback mode, transmitted data is echoed back to the receiver.
 */
Error setLoopbackState (uint8_t port, bool enable);

/** @} */

/**
 * @defgroup DriverFifo FIFO Control
 * @brief Functions for managing transmit and receive FIFO buffers.
 * @{
 */

/**
 * @brief Enable or disable the transmit/receive FIFO.
 *
 * Controls the FIFO (First-In-First-Out) buffer functionality for both
 * transmit and receive operations. The XR17V358 provides 128-byte FIFOs
 * for both directions. Enabling FIFO reduces interrupt overhead for
 * bulk data transfers.
 *
 * @param[in] port Port number [0, MAX_NUM_PORTS].
 * @param[in] enable true to enable FIFO mode, false to disable (1-byte mode).
 *
 * @retval ERROR_SUCCESS FIFO state changed successfully.
 * @retval ERROR_INVALID_PORT Port number is out of valid range.
 * @retval ERROR_NULL_REGISTER_BASE Register base address not initialized.
 *
 * @note FIFO mode is recommended for most applications.
 * @note When FIFO is disabled, the port operates in 1-byte TX/RX mode.
 *
 * @see getFifoState
 */
Error setFifoState (uint8_t port, bool enable);

/**
 * @brief Query whether the transmit/receive FIFO is enabled.
 *
 * Returns the current FIFO enable state for the specified port.
 *
 * @param[in] port Port number [0, MAX_NUM_PORTS].
 * @param[out] state Pointer to bool to receive FIFO state.
 *            true = FIFO enabled, false = FIFO disabled (1-byte mode).
 *
 * @retval ERROR_SUCCESS FIFO state retrieved successfully.
 * @retval ERROR_INVALID_PORT Port number is out of valid range.
 * @retval ERROR_NULL_PTR state pointer is NULL.
 * @retval ERROR_NULL_REGISTER_BASE Register base address not initialized.
 *
 * @see setFifoState
 */
Error getFifoState (uint8_t port, bool *state);

/** @} */

/**
 * @defgroup DriverTesting Testing and Debugging
 * @brief Functions for testing and initialization support.
 * @{
 */

#ifdef SERIAL_DRIVER_TESTING

/**
 * @brief Set the register base address for unit testing.
 *
 * Configures the base address of the device register memory map. This function
 * is only available when SERIAL_DRIVER_TESTING is defined, enabling unit
 * tests to provide a mock memory location instead of real hardware MMIO.
 *
 * @param[in] base_addr Register base address. Set to 0 to clear (disable).
 *
 * @note This function should only be used in test environments.
 * @note Production code should not call this function.
 *
 * @warning This is a testing interface and not part of the production API.
 */
void serialDriverSetRegisterBaseForTest (uintptr_t base_addr);

#endif

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* SERIAL_DRIVER_H */
