#include "x17v358_regs.h"

#include <gtest/gtest.h>

TEST (X17V358Regs, ChannelStructIsEightBytes)
{
  EXPECT_EQ (sizeof (x17v358_channel_regs_t), 8u);
}

TEST (X17V358Regs, EnhancedStructIsEightBytes)
{
  EXPECT_EQ (sizeof (xr17v358_enhanced_regs_t), 8u);
}

TEST (X17V358Regs, Mmio8ReadWrite)
{
  volatile uint8_t reg = 0u;

  mmio8_write (&reg, 0xA5u);

  EXPECT_EQ (mmio8_read (&reg), 0xA5u);
}

TEST (X17V358Regs, Mmio32SwappedReadWrite)
{
  volatile uint32_t reg = 0u;

  mmio32_write_swapped (&reg, 0x11223344u);

  EXPECT_EQ (reg, 0x44332211u);
  EXPECT_EQ (mmio32_read_swapped (&reg), 0x11223344u);
}

TEST (X17V358Regs, RxTxHelpersUseSharedFifoRegister)
{
  x17v358_channel_regs_t regs = {};

  x17v358_tx_thr_write (&regs, 0x5Au);
  EXPECT_EQ (x17v358_rx_fifo_read (&regs), 0x5Au);

  x17v358_tx_fifo_write (&regs, 0xC3u);
  EXPECT_EQ (regs.rhr_thr_dll.raw, 0xC3u);
}

TEST (X17V358Regs, BitfieldAccessReflectsRawValue)
{
  x17v358_reg8_bits_t reg = {};

  reg.raw = 0xA5u;

  EXPECT_EQ (static_cast<unsigned int> (reg.bits.bit0), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.bit2), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.bit5), 1u);
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

TEST (X17V358Regs, FcrIsrBitfieldAccessReflectsRawValue)
{
  x17v358_fcr_isr_reg_t reg = {};

  reg.raw = 0x8Fu;

  EXPECT_EQ (static_cast<unsigned int> (reg.fcr_bits.fifo_enable), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.fcr_bits.dma_mode_select), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.fcr_bits.rx_trigger_level), 2u);
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

TEST (X17V358Regs, MsrBitfieldAccessReflectsRawValue)
{
  x17v358_msr_reg_t reg = {};

  reg.raw = 0xF0u;

  EXPECT_EQ (static_cast<unsigned int> (reg.bits.cts), 1u);
  EXPECT_EQ (static_cast<unsigned int> (reg.bits.dcd), 1u);
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
