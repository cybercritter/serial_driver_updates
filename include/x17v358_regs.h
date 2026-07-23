/**
 * @file x17v358_regs.h
 * @brief Register map and definitions for MaxLinear XR17V358 8-channel PCIe UART.
 *
 * This header defines the complete register map for the MaxLinear XR17V358
 * 8-channel PCIe UART device. Each channel occupies a 1 KiB (0x400-byte)
 * memory window in BAR0, with channel-specific UART registers and global
 * device registers.
 *
 * @section layout Memory Layout
 * - **Channel 0-7 Base**: BAR0 + (channel_number * 0x400)
 * - **Global Registers Base**: BAR0 + (8 * 0x400) = BAR0 + 0x2000
 *
 * @section registers Register Types
 * Registers are organized as type-safe unions combining:
 * - **raw**: Direct access to the 8-bit register value
 * - **bits**: Named bitfield access for individual signal control
 *
 * @section access_rules Access Rules
 * - All registers are 8-bit wide
 * - Some registers are read-only (indicated in documentation)
 * - Most registers are read-write
 * - Access should use volatile qualifiers to prevent compiler optimization
 *
 * @author Serial Driver Development Team
 * @date 2026-07-20
 */

#ifndef X17V358_REGS_H
#define X17V358_REGS_H

#include <stdint.h>

/**
 * @defgroup DeviceConsts Device Constants
 * @brief Hardware-specific constants.
 * @{
 */

/** @brief Maximum number of UART channels per device. */
#define MAX_NUM_PORTS 12

/** @} */

/**
 * @defgroup RegisterTypes Register Type Definitions
 * @brief Union types for type-safe register access.
 * @{
 */

/**
 * @brief Generic 8-bit register with individual bit access.
 *
 * Provides both raw register access and named bitfield access for an 8-bit
 * register containing 8 independent single-bit flags or status indicators.
 */
typedef union
{
  volatile uint8_t raw;              /**< @brief Raw 8-bit register access. */
  struct
  {
    volatile uint8_t bit0 : 1;       /**< @brief Bit 0 */
    volatile uint8_t bit1 : 1;       /**< @brief Bit 1 */
    volatile uint8_t bit2 : 1;       /**< @brief Bit 2 */
    volatile uint8_t bit3 : 1;       /**< @brief Bit 3 */
    volatile uint8_t bit4 : 1;       /**< @brief Bit 4 */
    volatile uint8_t bit5 : 1;       /**< @brief Bit 5 */
    volatile uint8_t bit6 : 1;       /**< @brief Bit 6 */
    volatile uint8_t bit7 : 1;       /**< @brief Bit 7 */
  } bits;                             /**< @brief Individual bit fields. */
} x17v358_reg8_bits_t;

/**
 * @brief Enhanced Function Register (EFR) - Extended capabilities control.
 *
 * Offset: 0x02 (when DLAB=0 in LCR)
 * Access: Read/Write
 *
 * Controls extended and enhanced UART functions including software flow control,
 * automatic hardware flow control, and special character detection.
 */
typedef union
{
  volatile uint8_t raw;              /**< @brief Raw register access. */
  struct
  {
    volatile uint8_t sw_flow_control : 4;        /**< @brief Software flow control mode. */
    volatile uint8_t enhanced_functions : 1;     /**< @brief Enable enhanced functions. */
    volatile uint8_t special_char_detect : 1;    /**< @brief Enable special character detection. */
    volatile uint8_t auto_rts_flow_control : 1;  /**< @brief Automatic RTS flow control. */
    volatile uint8_t auto_cts_flow_control : 1;  /**< @brief Automatic CTS flow control. */
  } bits;                             /**< @brief Bitfield access. */
} xr17v358_efr_reg_t;

/**
 * @brief Line Control Register (LCR) - Serial format and baud rate configuration.
 *
 * Offset: 0x03
 * Access: Read/Write
 *
 * Controls word length, stop bits, parity, and divisor latch access.
 * The DLAB bit enables access to the divisor latch for baud rate programming.
 */
typedef union
{
  volatile uint8_t raw;              /**< @brief Raw register access. */
  struct
  {
    volatile uint8_t word_length : 2;      /**< @brief Word length (00=5, 01=6, 10=7, 11=8 bits). */
    volatile uint8_t stop_bits : 1;        /**< @brief Stop bits (0=1, 1=2). */
    volatile uint8_t parity_enable : 1;    /**< @brief Enable parity checking. */
    volatile uint8_t even_parity : 1;      /**< @brief 0=odd parity, 1=even parity. */
    volatile uint8_t stick_parity : 1;     /**< @brief Force parity bit to specific value. */
    volatile uint8_t break_control : 1;    /**< @brief Force BREAK condition. */
    volatile uint8_t dlab : 1;             /**< @brief Divisor Latch Access Bit. */
  } bits;                             /**< @brief Bitfield access. */
} x17v358_lcr_reg_t;

/**
 * @brief Divisor Latch Fraction Register (DLD) - Fractional baud divisor.
 *
 * Offset: 0x02 (when DLAB=1 in LCR)
 * Access: Read/Write
 *
 * Holds the fractional baud-rate divisor in sixteenths. DLD[3:0] is the
 * fraction field; upper bits are reserved and should be preserved.
 */
typedef union
{
  volatile uint8_t raw;              /**< @brief Raw register access. */
  struct
  {
    volatile uint8_t fraction : 4;   /**< @brief Fractional divisor, in 1/16 steps. */
    volatile uint8_t reserved : 4;   /**< @brief Reserved bits. */
  } bits;                             /**< @brief Bitfield access. */
} x17v358_dld_reg_t;

/**
 * @brief Line Status Register (LSR) - Transmit/receive status and error flags.
 *
 * Offset: 0x05
 * Access: Read-only
 *
 * Indicates transmitter and receiver status, including FIFO empty conditions
 * and serial communication errors (overrun, parity, framing, break).
 */
typedef union
{
  volatile uint8_t raw;              /**< @brief Raw register access. */
  struct
  {
    volatile uint8_t data_ready : 1;        /**< @brief Receiver data available. */
    volatile uint8_t overrun_error : 1;     /**< @brief Receiver overrun error. */
    volatile uint8_t parity_error : 1;      /**< @brief Receiver parity error. */
    volatile uint8_t framing_error : 1;     /**< @brief Receiver framing error. */
    volatile uint8_t break_interrupt : 1;   /**< @brief Break condition detected. */
    volatile uint8_t thr_empty : 1;         /**< @brief Transmit Holding Register empty. */
    volatile uint8_t tx_empty : 1;          /**< @brief Transmitter idle. */
    volatile uint8_t fifo_error : 1;        /**< @brief FIFO error occurred. */
  } bits;                             /**< @brief Bitfield access. */
} x17v358_lsr_reg_t;

/**
 * @brief Modem Control Register (MCR) - RTS/DTR/loopback/flow control.
 *
 * Offset: 0x04
 * Access: Read/Write
 *
 * Controls modem output signals (RTS, DTR), loopback mode, and automatic
 * flow control enable.
 */
typedef union
{
  volatile uint8_t raw;              /**< @brief Raw register access. */
  struct
  {
    volatile uint8_t dtr : 1;                      /**< @brief Data Terminal Ready output. */
    volatile uint8_t rts : 1;                      /**< @brief Request To Send output. */
    volatile uint8_t out1 : 1;                     /**< @brief Output 1 (user-defined). */
    volatile uint8_t out2 : 1;                     /**< @brief Output 2 (interrupt enable). */
    volatile uint8_t loopback : 1;                 /**< @brief Enable internal loopback mode. */
    volatile uint8_t auto_flow_control_enable : 1; /**< @brief Enable automatic flow control. */
    volatile uint8_t reserved_6 : 1;               /**< @brief Reserved bit. */
    volatile uint8_t reserved_7 : 1;               /**< @brief Reserved bit. */
  } bits;                             /**< @brief Bitfield access. */
} x17v358_mcr_reg_t;

/**
 * @brief Modem Status Register (MSR) - CTS/DSR/DCD/RI status.
 *
 * Offset: 0x06
 * Access: Read-only
 *
 * Provides status of modem input signals and change indicators for flow
 * control and connection status monitoring.
 */
typedef union
{
  volatile uint8_t raw;              /**< @brief Raw register access. */
  struct
  {
    volatile uint8_t delta_cts : 1;          /**< @brief CTS changed since last read. */
    volatile uint8_t delta_dsr : 1;          /**< @brief DSR changed since last read. */
    volatile uint8_t trailing_edge_ri : 1;   /**< @brief RI trailing edge detected. */
    volatile uint8_t delta_dcd : 1;          /**< @brief DCD changed since last read. */
    volatile uint8_t cts : 1;                /**< @brief Current CTS input state. */
    volatile uint8_t dsr : 1;                /**< @brief Current DSR input state. */
    volatile uint8_t ri : 1;                 /**< @brief Current RI input state. */
    volatile uint8_t dcd : 1;                /**< @brief Current DCD input state. */
  } bits;                             /**< @brief Bitfield access. */
} x17v358_msr_reg_t;

/**
 * @brief Scratch Pad Register (SPR) - General-purpose 8-bit storage.
 *
 * Offset: 0x07
 * Access: Read/Write
 *
 * General-purpose read/write register for temporary storage or device identification.
 */
typedef union
{
  volatile uint8_t raw;              /**< @brief Raw register access. */
  struct
  {
    volatile uint8_t data : 8;       /**< @brief 8-bit data storage. */
  } bits;                             /**< @brief Bitfield access. */
} x17v358_spr_reg_t;

/**
 * @brief Interrupt Enable Register (IER) - UART interrupt source control.
 *
 * Offset: 0x01 (when DLAB=0 in LCR)
 * Access: Read/Write
 *
 * Controls which UART events generate interrupt signals. Interrupts can be
 * generated for RX data, TX completion, status changes, and flow control events.
 */
typedef union
{
  volatile uint8_t raw;              /**< @brief Raw register access. */
  struct
  {
    volatile uint8_t rx_data_available_int_en : 1;          /**< @brief RX data available interrupt enable. */
    volatile uint8_t tx_holding_register_empty_int_en : 1;  /**< @brief TX holding register empty interrupt enable. */
    volatile uint8_t rx_line_status_int_en : 1;             /**< @brief RX line status interrupt enable. */
    volatile uint8_t modem_status_int_en : 1;               /**< @brief Modem status interrupt enable. */
    volatile uint8_t sleep_mode_int_en : 1;                 /**< @brief Sleep mode interrupt enable. */
    volatile uint8_t xoff_int_en : 1;                       /**< @brief Xoff character interrupt enable. */
    volatile uint8_t rts_cts_int_en : 1;                    /**< @brief RTS/CTS interrupt enable. */
    volatile uint8_t cts_rts_int_en : 1;                    /**< @brief CTS/RTS interrupt enable. */
  } bits;                             /**< @brief Bitfield access. */
} x17v358_ier_reg_t;

/**
 * @brief FIFO Control/Interrupt Status Register (FCR/ISR) - Dual-purpose register.
 *
 * Offset: 0x02
 * Access: FCR is write-only, ISR is read-only (same register)
 *
 * Provides both write-only FIFO control and read-only interrupt status.
 * When reading, provides interrupt identification; when writing, controls FIFO.
 */
typedef union
{
  volatile uint8_t raw;              /**< @brief Raw register access. */
  struct
  {
    volatile uint8_t interrupt_pending_n : 1;    /**< @brief 1=no interrupt pending, 0=interrupt pending. */
    volatile uint8_t interrupt_id : 3;           /**< @brief Interrupt source identification. */
    volatile uint8_t timeout_interrupt_pending : 1; /**< @brief RX timeout interrupt pending. */
    volatile uint8_t reserved_5 : 1;             /**< @brief Reserved. */
    volatile uint8_t fifo_enabled : 2;           /**< @brief FIFO enabled status. */
  } isr_bits;                         /**< @brief Interrupt Status (read). */
  struct
  {
    volatile uint8_t fifo_enable : 1;          /**< @brief Enable TX/RX FIFO (1=128-byte FIFO, 0=1-byte). */
    volatile uint8_t rx_fifo_reset : 1;        /**< @brief Clear RX FIFO (self-clearing). */
    volatile uint8_t tx_fifo_reset : 1;        /**< @brief Clear TX FIFO (self-clearing). */
    volatile uint8_t dma_mode_select : 1;      /**< @brief DMA mode select. */
    volatile uint8_t reserved_4_5 : 2;         /**< @brief Reserved. */
    volatile uint8_t rx_trigger_level : 2;     /**< @brief RX FIFO trigger level. */
  } fcr_bits;                         /**< @brief FIFO Control (write). */
} x17v358_fcr_isr_reg_t;

/** @} */

/**
 * @defgroup ChannelRegs Channel Register Map
 * @brief Per-channel UART register structure.
 * @{
 */

/**
 * @brief Per-channel UART register map for one channel.
 *
 * This structure overlays exactly 8 bytes (one cache line) starting at
 * (BAR0_virt + channel * 0x400). Each channel has independent copies
 * of these registers.
 *
 * @note All fields must be accessed as volatile to prevent compiler optimization.
 * @note Register offsets:
 *   - 0x00: RHR/THR/DLL
 *   - 0x01: IER/DLM
 *   - 0x02: ISR/FCR
 *   - 0x03: LCR
 *   - 0x04: MCR
 *   - 0x05: LSR
 *   - 0x06: MSR
 *   - 0x07: SPR
 */
typedef struct
{
  x17v358_reg8_bits_t rhr_thr_dll;   /**< @brief RX Hold / TX Hold / Divisor Latch Low. */
  x17v358_ier_reg_t ier_dlh;         /**< @brief Interrupt Enable / Divisor Latch High. */
  x17v358_fcr_isr_reg_t isr_fcr;     /**< @brief Interrupt Status / FIFO Control. */
  x17v358_lcr_reg_t lcr;             /**< @brief Line Control. */
  x17v358_mcr_reg_t mcr;             /**< @brief Modem Control. */
  x17v358_lsr_reg_t lsr;             /**< @brief Line Status. */
  x17v358_msr_reg_t msr;             /**< @brief Modem Status. */
  x17v358_spr_reg_t spr;             /**< @brief Scratch Pad. */
} x17v358_channel_regs_t;

/** @} */

/**
 * @defgroup EnhancedRegs Enhanced/Global Register Map
 * @brief Enhanced UART and global device registers.
 * @{
 */

/**
 * @brief Enhanced UART register map for extended functionality.
 *
 * Provides access to advanced features including software flow control
 * configuration and XON/XOFF character specification.
 */
typedef struct
{
  x17v358_reg8_bits_t reserved_00;   /**< @brief Reserved register. */
  x17v358_reg8_bits_t reserved_01;   /**< @brief Reserved register. */
  xr17v358_efr_reg_t efr;            /**< @brief Enhanced Function Register. */
  x17v358_reg8_bits_t reserved_03;   /**< @brief Reserved register. */
  x17v358_reg8_bits_t xon1;          /**< @brief XON character 1. */
  x17v358_reg8_bits_t xon2;          /**< @brief XON character 2. */
  x17v358_reg8_bits_t xoff1;         /**< @brief XOFF character 1. */
  x17v358_reg8_bits_t xoff2;         /**< @brief XOFF character 2. */
} xr17v358_enhanced_regs_t;

/** @} */

/**
 * @cond INTERNAL
 * Static assertions to verify register structure sizes.
 * The XR17V358 channel registers must be exactly 8 bytes to match the
 * hardware-defined register layout.
 */
#ifdef __cplusplus
static_assert (sizeof (x17v358_channel_regs_t) == 8u,
               "Unexpected x17v358 channel register size");
static_assert (sizeof (xr17v358_enhanced_regs_t) == 8u,
               "Unexpected xr17v358 enhanced register size");
#else
_Static_assert (sizeof (x17v358_channel_regs_t) == 8u,
                "Unexpected x17v358 channel register size");
_Static_assert (sizeof (xr17v358_enhanced_regs_t) == 8u,
                "Unexpected xr17v358 enhanced register size");
#endif
/** @endcond */

/**
 * @defgroup MmioHelpers MMIO Helper Functions
 * @brief Type-safe memory-mapped I/O access functions.
 * @{
 */

/**
 * @brief Read an 8-bit value from a memory-mapped I/O address.
 *
 * @param[in] addr Volatile pointer to the register address.
 *
 * @return The 8-bit value read from the specified address.
 *
 * @note Use this for all MMIO register reads to ensure consistent memory
 *       access patterns and prevent compiler optimization.
 */
static inline uint8_t
mmio8_read (const volatile void *addr)
{
  return *(const volatile uint8_t *)addr;
}

/**
 * @brief Write an 8-bit value to a memory-mapped I/O address.
 *
 * @param[in] addr Volatile pointer to the register address.
 * @param[in] value The 8-bit value to write.
 *
 * @note Use this for all MMIO register writes to ensure consistent memory
 *       access patterns and prevent compiler optimization.
 */
static inline void
mmio8_write (volatile void *addr, uint8_t value)
{
  *(volatile uint8_t *)addr = value;
}

/**
 * @brief Perform a 32-bit byte-swap (endian conversion).
 *
 * Swaps the byte order of a 32-bit value. Useful for converting between
 * native and big-endian representations.
 *
 * @param[in] value The value to byte-swap.
 *
 * @return The byte-swapped value.
 *
 * @note Uses compiler built-in __builtin_bswap32 for efficiency.
 */
static inline uint32_t
mmio32_bswap (uint32_t value)
{
  return __builtin_bswap32 (value);
}

/**
 * @brief Read a 32-bit value from a memory-mapped I/O address.
 *
 * @param[in] addr Volatile pointer to the register address.
 *
 * @return The 32-bit value read from the specified address.
 *
 * @note Use for 32-bit register access without byte-swapping.
 */
static inline uint32_t
mmio32_read (const volatile void *addr)
{
  return *(const volatile uint32_t *)addr;
}

/**
 * @brief Write a 32-bit value to a memory-mapped I/O address.
 *
 * @param[in] addr Volatile pointer to the register address.
 * @param[in] value The 32-bit value to write.
 *
 * @note Use for 32-bit register writes without byte-swapping.
 */
static inline void
mmio32_write (volatile void *addr, uint32_t value)
{
  *(volatile uint32_t *)addr = value;
}

/**
 * @brief Read a 32-bit value with byte-swap from a memory-mapped I/O address.
 *
 * Reads a 32-bit value and performs endian conversion. Useful for accessing
 * registers in devices with different endianness.
 *
 * @param[in] addr Volatile pointer to the register address.
 *
 * @return The 32-bit value read and byte-swapped.
 *
 * @see mmio32_write_swapped
 */
static inline uint32_t
mmio32_read_swapped (const volatile void *addr)
{
  return mmio32_bswap (mmio32_read (addr));
}

/**
 * @brief Write a 32-bit value with byte-swap to a memory-mapped I/O address.
 *
 * Performs endian conversion and writes a 32-bit value. Useful for writing
 * registers in devices with different endianness.
 *
 * @param[in] addr Volatile pointer to the register address.
 * @param[in] value The 32-bit value to byte-swap and write.
 *
 * @see mmio32_read_swapped
 */
static inline void
mmio32_write_swapped (volatile void *addr, uint32_t value)
{
  mmio32_write (addr, mmio32_bswap (value));
}

/**
 * @brief Read an 8-bit register value.
 *
 * Type-safe wrapper for reading an 8-bit register. Provides semantic clarity
 * compared to direct pointer dereference.
 *
 * @param[in] reg Pointer to volatile 8-bit register.
 *
 * @return The register value.
 */
static inline uint8_t
x17v358_read_reg8 (const volatile uint8_t *reg)
{
  return mmio8_read (reg);
}

/**
 * @brief Write an 8-bit register value.
 *
 * Type-safe wrapper for writing an 8-bit register. Provides semantic clarity
 * compared to direct pointer assignment.
 *
 * @param[in] reg Pointer to volatile 8-bit register.
 * @param[in] value The value to write.
 */
static inline void
x17v358_write_reg8 (volatile uint8_t *reg, uint8_t value)
{
  mmio8_write (reg, value);
}

/**
 * @brief Read a byte from the RX FIFO (RHR register).
 *
 * Reads one byte from the receive buffer. This register is read-only and
 * contains the oldest byte received when data is available.
 *
 * @param[in] regs Pointer to the channel register structure.
 *
 * @return The byte read from the RX FIFO.
 *
 * @note Check LSR.data_ready before calling to verify data availability.
 */
static inline uint8_t
x17v358_rx_fifo_read (const volatile x17v358_channel_regs_t *regs)
{
  return mmio8_read (&regs->rhr_thr_dll.raw);
}

/**
 * @brief Write a byte to the TX holding register (THR).
 *
 * Transmits one byte by writing to the transmit holding register. Data is
 * automatically transferred to the TX FIFO.
 *
 * @param[in] regs Pointer to the channel register structure.
 * @param[in] value The byte to transmit.
 *
 * @note Check LSR.thr_empty before calling to verify the holding register
 *       is ready to accept new data.
 */
static inline void
x17v358_tx_thr_write (volatile x17v358_channel_regs_t *regs, uint8_t value)
{
  mmio8_write (&regs->rhr_thr_dll.raw, value);
}

/**
 * @brief Write a byte to the TX FIFO.
 *
 * Transmits one byte by writing to the TX FIFO. This is equivalent to
 * x17v358_tx_thr_write() as they both access the same register.
 *
 * @param[in] regs Pointer to the channel register structure.
 * @param[in] value The byte to transmit.
 *
 * @see x17v358_tx_thr_write
 */
static inline void
x17v358_tx_fifo_write (volatile x17v358_channel_regs_t *regs, uint8_t value)
{
  x17v358_tx_thr_write (regs, value);
}

/** @} */

#endif /* X17V358_REGS_H */
