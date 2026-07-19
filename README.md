# serial_driver_updates

## Build

```sh
cmake -S . -B build
cmake --build build
```

## Test

```sh
cmake -S . -B build -DSERIAL_DRIVER_ENABLE_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Coverage

Coverage requires GCC or Clang. If `gcovr` is installed, a `coverage` target is created.

```sh
cmake -S . -B build \
    -DSERIAL_DRIVER_ENABLE_TESTS=ON \
    -DSERIAL_DRIVER_ENABLE_COVERAGE=ON
cmake --build build
cmake --build build --target coverage
```

## x17v358 Register Header

The project now includes `include/x17v358_regs.h` with:

- `x17v358_channel_regs_t` register map for one UART channel
- `mmio8_read` and `mmio8_write` helpers for 8-bit MMIO access
- `mmio32_read`, `mmio32_write`, `mmio32_read_swapped`, and `mmio32_write_swapped`
- `x17v358_rx_fifo_read`, `x17v358_tx_thr_write`, and `x17v358_tx_fifo_write` helpers for RX/TX FIFO byte access
