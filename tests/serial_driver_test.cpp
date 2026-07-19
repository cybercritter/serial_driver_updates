#include "serial_driver.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>

#include "x17v358_regs.h"

namespace {
constexpr uint8_t kMinPort = 0u;
constexpr uint8_t kMaxPort = 12u;
constexpr uint8_t kInvalidPort = kMaxPort + 1u;
constexpr size_t kPortCount = static_cast<size_t>(kMaxPort) + 1u;
constexpr size_t kPortStride = 0x400u;

class SerialDriverTest : public ::testing::Test {
 protected:
  void SetUp() override {
    mmio.fill(std::byte{0u});
    serialDriverSetRegisterBaseForTest(
        reinterpret_cast<uintptr_t>(mmio.data()));
  }

  void TearDown() override { serialDriverSetRegisterBaseForTest(0u); }

  volatile x17v358_channel_regs_t* regsFor(uint8_t port) {
    return reinterpret_cast<volatile x17v358_channel_regs_t*>(
        mmio.data() + (static_cast<size_t>(port) * kPortStride));
  }

  std::array<std::byte, kPortCount * kPortStride> mmio{};
};
}  // namespace

TEST_F(SerialDriverTest, PortInitializationConfiguresExpectedDefaults) {
  for (uint8_t port = kMinPort; port <= kMaxPort; ++port) {
    volatile x17v358_channel_regs_t* regs = regsFor(port);

    regs->ier_dlh.raw = 0xFFu;
    regs->mcr.raw = 0xFFu;
    regs->isr_fcr.raw = 0u;

    EXPECT_EQ(portInitialization(port), ERROR_SUCCESS);
    EXPECT_EQ(regs->ier_dlh.raw, 0u);
    EXPECT_EQ(static_cast<unsigned int>(regs->mcr.bits.loopback), 0u);
    EXPECT_EQ(static_cast<unsigned int>(regs->mcr.bits.rts), 0u);
    EXPECT_EQ(static_cast<unsigned int>(regs->isr_fcr.fcr_bits.fifo_enable),
              1u);
  }
}

TEST_F(SerialDriverTest, PortInitializationRejectsInvalidPort) {
  EXPECT_EQ(portInitialization(kInvalidPort), ERROR_INVALID_PARAM);
}

TEST_F(SerialDriverTest, DisableAllInterruptsClearsAllInterruptBits) {
  for (uint8_t port = kMinPort; port <= kMaxPort; ++port) {
    volatile x17v358_channel_regs_t* regs = regsFor(port);
    regs->ier_dlh.raw = 0xFFu;

    EXPECT_EQ(disableAllInterrupts(port), ERROR_SUCCESS);
    EXPECT_EQ(regs->ier_dlh.raw, 0u);
  }
}

TEST_F(SerialDriverTest, DisableAllInterruptsRejectsInvalidPort) {
  EXPECT_EQ(disableAllInterrupts(kInvalidPort), ERROR_INVALID_PARAM);
}

TEST_F(SerialDriverTest, SetDiscreteStateUpdatesRtsBit) {
  for (uint8_t port = kMinPort; port <= kMaxPort; ++port) {
    volatile x17v358_channel_regs_t* regs = regsFor(port);

    EXPECT_EQ(setDiscreteState(port, true), ERROR_SUCCESS);
    EXPECT_EQ(static_cast<unsigned int>(regs->mcr.bits.rts), 1u);

    EXPECT_EQ(setDiscreteState(port, false), ERROR_SUCCESS);
    EXPECT_EQ(static_cast<unsigned int>(regs->mcr.bits.rts), 0u);
  }
}

TEST_F(SerialDriverTest, GetDiscreteStateReturnsCurrentRtsBit) {
  for (uint8_t port = kMinPort; port <= kMaxPort; ++port) {
    bool state = false;
    volatile x17v358_channel_regs_t* regs = regsFor(port);

    regs->mcr.bits.rts = 1u;

    EXPECT_EQ(getDiscreteState(port, &state), ERROR_SUCCESS);
    EXPECT_TRUE(state);
  }
}

TEST_F(SerialDriverTest, GetDiscreteStateRejectsNullState) {
  for (uint8_t port = kMinPort; port <= kMaxPort; ++port) {
    EXPECT_EQ(getDiscreteState(port, nullptr), ERROR_NULL_PTR);
  }
}

TEST_F(SerialDriverTest, SetLoopbackStateUpdatesLoopbackBit) {
  for (uint8_t port = kMinPort; port <= kMaxPort; ++port) {
    volatile x17v358_channel_regs_t* regs = regsFor(port);

    EXPECT_EQ(setLoopbackState(port, true), ERROR_SUCCESS);
    EXPECT_EQ(static_cast<unsigned int>(regs->mcr.bits.loopback), 1u);

    EXPECT_EQ(setLoopbackState(port, false), ERROR_SUCCESS);
    EXPECT_EQ(static_cast<unsigned int>(regs->mcr.bits.loopback), 0u);
  }
}

TEST_F(SerialDriverTest, SetFifoStateAndGetFifoStateRoundTrip) {
  for (uint8_t port = kMinPort; port <= kMaxPort; ++port) {
    bool enabled = false;
    volatile x17v358_channel_regs_t* regs = regsFor(port);

    EXPECT_EQ(setFifoState(port, true), ERROR_SUCCESS);
    EXPECT_EQ(static_cast<unsigned int>(regs->isr_fcr.fcr_bits.fifo_enable),
              1u);

    EXPECT_EQ(getFifoState(port, &enabled), ERROR_SUCCESS);
    EXPECT_TRUE(enabled);

    EXPECT_EQ(setFifoState(port, false), ERROR_SUCCESS);
    EXPECT_EQ(getFifoState(port, &enabled), ERROR_SUCCESS);
    EXPECT_FALSE(enabled);
  }
}

TEST_F(SerialDriverTest, GetFifoStateRejectsNullState) {
  for (uint8_t port = kMinPort; port <= kMaxPort; ++port) {
    EXPECT_EQ(getFifoState(port, nullptr), ERROR_NULL_PTR);
  }
}

TEST_F(SerialDriverTest, SetDiscreteStateRejectsInvalidPort) {
  EXPECT_EQ(setDiscreteState(kInvalidPort, true), ERROR_INVALID_PARAM);
  EXPECT_EQ(setDiscreteState(kInvalidPort, false), ERROR_INVALID_PARAM);
}

TEST_F(SerialDriverTest, GetDiscreteStateRejectsInvalidPort) {
  bool state = false;
  EXPECT_EQ(getDiscreteState(kInvalidPort, &state), ERROR_INVALID_PARAM);
}

TEST_F(SerialDriverTest, SetLoopbackStateRejectsInvalidPort) {
  EXPECT_EQ(setLoopbackState(kInvalidPort, true), ERROR_INVALID_PARAM);
  EXPECT_EQ(setLoopbackState(kInvalidPort, false), ERROR_INVALID_PARAM);
}

TEST_F(SerialDriverTest, GetFifoStateRejectsInvalidPort) {
  bool state = false;
  EXPECT_EQ(getFifoState(kInvalidPort, &state), ERROR_INVALID_PARAM);
}

TEST_F(SerialDriverTest, SetFifoStateRejectsInvalidPort) {
  EXPECT_EQ(setFifoState(kInvalidPort, true), ERROR_INVALID_PARAM);
  EXPECT_EQ(setFifoState(kInvalidPort, false), ERROR_INVALID_PARAM);
}

TEST_F(SerialDriverTest, FunctionsReturnNullPtrWhenRegisterBaseIsZero) {
  constexpr uint8_t kNullPort = 0u;
  bool state = false;

  serialDriverSetRegisterBaseForTest(0u);

  EXPECT_EQ(disableAllInterrupts(kNullPort), ERROR_NULL_PTR);
  EXPECT_EQ(setDiscreteState(kNullPort, true), ERROR_NULL_PTR);
  EXPECT_EQ(getDiscreteState(kNullPort, &state), ERROR_NULL_PTR);
  EXPECT_EQ(setLoopbackState(kNullPort, true), ERROR_NULL_PTR);
  EXPECT_EQ(setFifoState(kNullPort, true), ERROR_NULL_PTR);
  EXPECT_EQ(getFifoState(kNullPort, &state), ERROR_NULL_PTR);
}
