# RISC-V Dual-Hart Multi-Thread Samples

This app now builds two binaries for `riscv64-xs-dual`:

- `mt-low`: low-pressure functional samples
- `mt-stress`: high-pressure communication stress samples

Both binaries reuse the existing dual-hart boot path in AM and branch on
`mhartid` in `main()`.

## Build

```sh
cd apps/mt
make ARCH=riscv64-xs-dual
```

The command above produces:

- `build/mt-low-riscv64-xs-dual.bin`
- `build/mt-stress-riscv64-xs-dual.bin`

To build only one variant with a specific baked-in sample:

```sh
make low ARCH=riscv64-xs-dual LOW_MAINARGS=split
make stress ARCH=riscv64-xs-dual STRESS_MAINARGS=queue
```

## Low-Pressure Samples

- `mainargs=split` or `1`
  - Minimal role split by `hartid`
  - Verifies per-hart TLS state from the shared dual-hart startup

- `mainargs=mailbox` or `2`
  - Ping-pong interaction with a shared mailbox
  - Checks request and reply ordering across the two harts

- `mainargs=atomic` or `3`
  - Compares a racy shared counter with `_atomic_add`
  - Demonstrates why atomic primitives are needed

- `mainargs=lock` or `4`
  - Uses a spin lock built on `_atomic_xchg`
  - Verifies mutual exclusion in a shared critical section

## Stress Samples

- `mainargs=queue` or `5`
  - Stresses bidirectional SPSC message queues with wrap-around
  - Keeps multiple requests in flight and checks both payload and token integrity

## Notes

- Use `ARCH=riscv64-xs-dual`.
- `hart 0` wakes `hart 1` through `_mpe_wakeup(1)`.
- Shared-state handshakes in the samples use explicit `fence rw, rw` pairs for RISC-V WMO.
- On success, `hart 0` prints `PASS` and exits the simulation.
