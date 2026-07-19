#include <gtest/gtest.h>

#include <cstdint>

extern "C" {
#include "x17v358_regs.h"
}

TEST(X17V358Regs, ChannelStructIsEightBytes) {
  EXPECT_EQ(sizeof(x17v358_channel_regs_t), static_cast<size_t>(8));
}

TEST(X17V358Regs, Mmio8ReadWrite) {
  volatile uint8_t reg = 0;

  mmio8_write(&reg, 0xA5u);

  EXPECT_EQ(mmio8_read(&reg), static_cast<uint8_t>(0xA5u));
}

TEST(X17V358Regs, Mmio32SwappedReadWrite) {
  volatile uint32_t reg = 0;

  mmio32_write_swapped(&reg, 0x11223344u);
  EXPECT_EQ(reg, static_cast<uint32_t>(0x44332211u));

  reg = 0xA1B2C3D4u;
  EXPECT_EQ(mmio32_read_swapped(&reg), static_cast<uint32_t>(0xD4C3B2A1u));
}

TEST(X17V358Regs, RxTxHelpersUseSharedFifoRegister) {
  volatile x17v358_channel_regs_t regs = {};

  regs.rhr_thr_dll.raw = 0x5Au;
  EXPECT_EQ(x17v358_rx_fifo_read(&regs), static_cast<uint8_t>(0x5Au));

  x17v358_tx_thr_write(&regs, 0xC3u);
  EXPECT_EQ(regs.rhr_thr_dll.raw, static_cast<uint8_t>(0xC3u));
}

TEST(X17V358Regs, BitfieldAccessReflectsRawValue) {
  volatile x17v358_channel_regs_t regs = {};

  regs.lcr.raw = 0x80u;
  EXPECT_EQ(static_cast<unsigned int>(regs.lcr.bits.dlab), 1u);

  regs.lsr.raw = 0x21u;
  EXPECT_EQ(static_cast<unsigned int>(regs.lsr.bits.data_ready), 1u);
  EXPECT_EQ(static_cast<unsigned int>(regs.lsr.bits.thr_empty), 1u);
}
