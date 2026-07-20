#include "x17v358_regs.h"

#include <gtest/gtest.h>

// ========================================================================
// Structure Size Tests
// ========================================================================

TEST (X17V358Regs, ChannelStructIsEightBytes)
{
  EXPECT_EQ (sizeof (x17v358_channel_regs_t), 8u);
}

TEST (X17V358Regs, EnhancedStructIsEightBytes)
{
  EXPECT_EQ (sizeof (xr17v358_enhanced_regs_t), 8u);
}

// ========================================================================
// MMIO 8-bit Read/Write Tests
// ========================================================================

TEST (X17V358Regs, Mmio8ReadWrite)
{
  volatile uint8_t reg = 0u;

  mmio8_write (&reg, 0xA5u);

  EXPECT_EQ (mmio8_read (&reg), 0xA5u);
}

TEST (X17V358Regs, Mmio8WriteZero)
{
  volatile uint8_t reg = 0xFFu;

  mmio8_write (&reg, 0x00u);

  EXPECT_EQ (mmio8_read (&reg), 0x00u);
}

TEST (X17V358Regs, Mmio8WriteAllOnes)
{
  volatile uint8_t reg = 0x00u;

  mmio8_write (&reg, 0xFFu);

  EXPECT_EQ (mmio8_read (&reg), 0xFFu);
}

TEST (X17V358Regs, Mmio8MultipleWrites)
{
  volatile uint8_t reg = 0u;

  mmio8_write (&reg, 0x12u);
  EXPECT_EQ (mmio8_read (&reg), 0x12u);

  mmio8_write (&reg, 0x34u);
  EXPECT_EQ (mmio8_read (&reg), 0x34u);

  mmio8_write (&reg, 0x56u);
  EXPECT_EQ (mmio8_read (&reg), 0x56u);
}

// ========================================================================
// MMIO 32-bit Read/Write Tests
// ========================================================================

TEST (X17V358Regs, Mmio32ReadWrite)
{
  volatile uint32_t reg = 0u;

  mmio32_write (&reg, 0x12345678u);

  EXPECT_EQ (mmio32_read (&reg), 0x12345678u);
}

TEST (X17V358Regs, Mmio32WriteZero)
{
  volatile uint32_t reg = 0xFFFFFFFFu;

  mmio32_write (&reg, 0x00000000u);

  EXPECT_EQ (mmio32_read (&reg), 0x00000000u);
}

TEST (X17V358Regs, Mmio32WriteAllOnes)
{
  volatile uint32_t reg = 0x00000000u;

  mmio32_write (&reg, 0xFFFFFFFFu);

  EXPECT_EQ (mmio32_read (&reg), 0xFFFFFFFFu);
}

TEST (X17V358Regs, Mmio32Bswap)
{
  uint32_t value = 0x12345678u;

  EXPECT_EQ (mmio32_bswap (value), 0x78563412u);
}

TEST (X17V358Regs, Mmio32BswapAllZeros)
{
  EXPECT_EQ (mmio32_bswap (0x00000000u), 0x00000000u);
}

TEST (X17V358Regs, Mmio32BswapAllOnes)
{
  EXPECT_EQ (mmio32_bswap (0xFFFFFFFFu), 0xFFFFFFFFu);
}

TEST (X17V358Regs, Mmio32BswapSingleByte)
{
  EXPECT_EQ (mmio32_bswap (0x000000FFu), 0xFF000000u);
  EXPECT_EQ (mmio32_bswap (0x0000FF00u), 0x00FF0000u);
  EXPECT_EQ (mmio32_bswap (0x00FF0000u), 0x0000FF00u);
  EXPECT_EQ (mmio32_bswap (0xFF000000u), 0x000000FFu);
}

// ========================================================================
// MMIO 32-bit Swapped Read/Write Tests
// ========================================================================

TEST (X17V358Regs, Mmio32SwappedReadWrite)
{
  volatile uint32_t reg = 0u;

  mmio32_write_swapped (&reg, 0x11223344u);

  EXPECT_EQ (reg, 0x44332211u);
  EXPECT_EQ (mmio32_read_swapped (&reg), 0x11223344u);
}

TEST (X17V358Regs, Mmio32SwappedMultipleWrites)
{
  volatile uint32_t reg = 0u;

  mmio32_write_swapped (&reg, 0x12345678u);
  EXPECT_EQ (mmio32_read_swapped (&reg), 0x12345678u);

  mmio32_write_swapped (&reg, 0xAABBCCDDu);
  EXPECT_EQ (mmio32_read_swapped (&reg), 0xAABBCCDDu);

  mmio32_write_swapped (&reg, 0x00000000u);
  EXPECT_EQ (mmio32_read_swapped (&reg), 0x00000000u);
}

TEST (X17V358Regs, Mmio32SwappedZeroValue)
{
  volatile uint32_t reg = 0xFFFFFFFFu;

  mmio32_write_swapped (&reg, 0x00000000u);

  EXPECT_EQ (reg, 0x00000000u);
  EXPECT_EQ (mmio32_read_swapped (&reg), 0x00000000u);
}

TEST (X17V358Regs, Mmio32SwappedAllOnesValue)
{
  volatile uint32_t reg = 0x00000000u;

  mmio32_write_swapped (&reg, 0xFFFFFFFFu);

  EXPECT_EQ (reg, 0xFFFFFFFFu);
  EXPECT_EQ (mmio32_read_swapped (&reg), 0xFFFFFFFFu);
}

// ========================================================================
// Register-specific Read/Write Helpers
// ========================================================================

TEST (X17V358Regs, X17V358ReadReg8)
{
  volatile uint8_t reg = 0x42u;

  EXPECT_EQ (x17v358_read_reg8 (&reg), 0x42u);
}

TEST (X17V358Regs, X17V358WriteReg8)
{
  volatile uint8_t reg = 0u;

  x17v358_write_reg8 (&reg, 0xBEu);

  EXPECT_EQ (reg, 0xBEu);
}

TEST (X17V358Regs, X17V358ReadWriteReg8Roundtrip)
{
  volatile uint8_t reg = 0u;

  x17v358_write_reg8 (&reg, 0xC0u);
  EXPECT_EQ (x17v358_read_reg8 (&reg), 0xC0u);

  x17v358_write_reg8 (&reg, 0x3Eu);
  EXPECT_EQ (x17v358_read_reg8 (&reg), 0x3Eu);
}

// ========================================================================
// FIFO and THR Helper Tests
// ========================================================================

TEST (X17V358Regs, RxTxHelpersUseSharedFifoRegister)
{
  x17v358_channel_regs_t regs = {};

  x17v358_tx_thr_write (&regs, 0x5Au);
  EXPECT_EQ (x17v358_rx_fifo_read (&regs), 0x5Au);

  x17v358_tx_fifo_write (&regs, 0xC3u);
  EXPECT_EQ (regs.rhr_thr_dll.raw, 0xC3u);
}

TEST (X17V358Regs, RxFifoReadReturnsCorrectByte)
{
  x17v358_channel_regs_t regs = {};

  regs.rhr_thr_dll.raw = 0xA5u;
  EXPECT_EQ (x17v358_rx_fifo_read (&regs), 0xA5u);

  regs.rhr_thr_dll.raw = 0x00u;
  EXPECT_EQ (x17v358_rx_fifo_read (&regs), 0x00u);

  regs.rhr_thr_dll.raw = 0xFFu;
  EXPECT_EQ (x17v358_rx_fifo_read (&regs), 0xFFu);
}

TEST (X17V358Regs, TxThrWriteUpdatesRegister)
{
  x17v358_channel_regs_t regs = {};

  x17v358_tx_thr_write (&regs, 0x42u);
  EXPECT_EQ (regs.rhr_thr_dll.raw, 0x42u);

  x17v358_tx_thr_write (&regs, 0xDEu);
  EXPECT_EQ (regs.rhr_thr_dll.raw, 0xDEu);
}

TEST (X17V358Regs, TxFifoWriteEquivalentToTxThr)
{
  x17v358_channel_regs_t regs1 = {};
  x17v358_channel_regs_t regs2 = {};

  x17v358_tx_thr_write (&regs1, 0x7Eu);
  x17v358_tx_fifo_write (&regs2, 0x7Eu);

  EXPECT_EQ (regs1.rhr_thr_dll.raw, regs2.rhr_thr_dll.raw);
}

// ========================================================================
// Bitfield Access Tests
// ========================================================================

TEST (X17V358Regs, BitfieldAccessReflectsRawValue)
{
  x17v358_reg8_bits_t reg = {};

  reg.raw = 0xA5u;

  EXPECT_EQ (static_cast<unsigned int> (reg.bits.bit0), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.bit2), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.bit5), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.bit7), 1u);
}

TEST (X17V358Regs, BitfieldZeroValue)
{
  x17v358_reg8_bits_t reg = {};

  reg.raw = 0x00u;

  EXPECT_EQ (static_cast<unsigned int> (reg.bits.bit0), 0u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.bit1), 0u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.bit7), 0u);
}

TEST (X17V358Regs, BitfieldAllOnesValue)
{
  x17v358_reg8_bits_t reg = {};

  reg.raw = 0xFFu;

  EXPECT_EQ (static_cast<unsigned int> (reg.bits.bit0), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.bit3), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.bit7), 1u);
}

TEST (X17V358Regs, IerBitfieldAccessReflectsRawValue)
{
  x17v358_ier_reg_t reg = {};

  reg.raw = 0xFFu;

  EXPECT_EQ (static_cast<unsigned int> (reg.bits.rx_data_available_int_en),
             1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.cts_rts_int_en), 1u);
}

TEST (X17V358Regs, IerBitfieldIndividualBits)
{
  x17v358_ier_reg_t reg = {};

  reg.raw = 0x01u;  // Only rx_data_available_int_en
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.rx_data_available_int_en),
             1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.tx_holding_register_empty_int_en), 0u);

  reg.raw = 0x00u;
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.rx_data_available_int_en),
             0u);
}

TEST (X17V358Regs, FcrIsrBitfieldAccessReflectsRawValue)
{
  x17v358_fcr_isr_reg_t reg = {};

  reg.raw = 0x8Fu;

  EXPECT_EQ (static_cast<unsigned int> (reg.fcr_bits.fifo_enable), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.fcr_bits.dma_mode_select), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.fcr_bits.rx_trigger_level), 2u);
}

TEST (X17V358Regs, FcrIsrFcrBitsIndividually)
{
  x17v358_fcr_isr_reg_t reg = {};

  reg.raw = 0x01u;  // fifo_enable
  EXPECT_EQ (static_cast<unsigned int> (reg.fcr_bits.fifo_enable), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.fcr_bits.rx_fifo_reset), 0u);

  reg.raw = 0x02u;  // rx_fifo_reset
  EXPECT_EQ (static_cast<unsigned int> (reg.fcr_bits.fifo_enable), 0u);
  EXPECT_EQ (static_cast<unsigned int> (reg.fcr_bits.rx_fifo_reset), 1u);
}

TEST (X17V358Regs, McrBitfieldAccessReflectsRawValue)
{
  x17v358_mcr_reg_t reg = {};

  reg.raw = 0x3Fu;

  EXPECT_EQ (static_cast<unsigned int> (reg.bits.rts), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.loopback), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.auto_flow_control_enable),
             1u);
}

TEST (X17V358Regs, McrBitfieldIndividualBits)
{
  x17v358_mcr_reg_t reg = {};

  reg.raw = 0x02u;  // rts
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.rts), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.loopback), 0u);

  reg.raw = 0x10u;  // loopback
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.rts), 0u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.loopback), 1u);
}

TEST (X17V358Regs, MsrBitfieldAccessReflectsRawValue)
{
  x17v358_msr_reg_t reg = {};

  reg.raw = 0xF0u;

  EXPECT_EQ (static_cast<unsigned int> (reg.bits.cts), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.dcd), 1u);
}

TEST (X17V358Regs, MsrBitfieldIndividualBits)
{
  x17v358_msr_reg_t reg = {};

  reg.raw = 0x10u;  // cts
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.cts), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.dcd), 0u);

  reg.raw = 0x80u;  // dcd
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.cts), 0u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.dcd), 1u);
}

// ========================================================================
// Line Status Register Tests
// ========================================================================

TEST (X17V358Regs, LsrBitfieldAccess)
{
  x17v358_lsr_reg_t reg = {};

  reg.raw = 0xFFu;

  EXPECT_EQ (static_cast<unsigned int> (reg.bits.data_ready), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.overrun_error), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.parity_error), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.framing_error), 1u);
}

TEST (X17V358Regs, LsrDataReadyBit)
{
  x17v358_lsr_reg_t reg = {};

  reg.raw = 0x01u;  // data_ready
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.data_ready), 1u);

  reg.raw = 0x00u;
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.data_ready), 0u);
}

// ========================================================================
// Enhanced Function Register Tests
// ========================================================================

TEST (X17V358Regs, EfrBitfieldAccess)
{
  xr17v358_efr_reg_t reg = {};

  reg.raw = 0xFFu;

  EXPECT_EQ (static_cast<unsigned int> (reg.bits.enhanced_functions), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.auto_rts_flow_control), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.auto_cts_flow_control), 1u);
}

TEST (X17V358Regs, EfrBitfieldZeroValue)
{
  xr17v358_efr_reg_t reg = {};

  reg.raw = 0x00u;

  EXPECT_EQ (static_cast<unsigned int> (reg.bits.enhanced_functions), 0u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.auto_rts_flow_control), 0u);
}

// ========================================================================
// Line Control Register Tests
// ========================================================================

TEST (X17V358Regs, LcrBitfieldAccess)
{
  x17v358_lcr_reg_t reg = {};

  reg.raw = 0xFFu;

  EXPECT_EQ (static_cast<unsigned int> (reg.bits.word_length), 3u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.stop_bits), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.parity_enable), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.dlab), 1u);
}

TEST (X17V358Regs, LcrDlabBit)
{
  x17v358_lcr_reg_t reg = {};

  reg.raw = 0x80u;  // dlab
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.dlab), 1u);

  reg.raw = 0x7Fu;
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.dlab), 0u);
}

// ========================================================================
// Scratch Pad Register Tests
// ========================================================================

TEST (X17V358Regs, SprReadWrite)
{
  x17v358_spr_reg_t reg = {};

  reg.raw = 0x42u;
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.data), 0x42u);

  reg.raw = 0xA5u;
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.data), 0xA5u);
}


TEST (X17V358Regs, SprBitfieldAccessReflectsRawValue)
{
  x17v358_spr_reg_t reg = {};

  reg.raw = 0x5Au;

  EXPECT_EQ (static_cast<unsigned int> (reg.bits.data), 0x5Au);
}

TEST (X17V358Regs, EnhancedEfrBitfieldAccessReflectsRawValue)
{
  xr17v358_efr_reg_t reg = {};

  reg.raw = 0xF5u;

  EXPECT_EQ (static_cast<unsigned int> (reg.bits.sw_flow_control), 0x5u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.enhanced_functions), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.auto_cts_flow_control), 1u);
}
