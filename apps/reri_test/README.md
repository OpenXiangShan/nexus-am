# RERI Software Injection Test

This application validates the RERI MMIO protocol and the UEC-to-NMI
notification path. It uses the shared AM NMI support through
`_nmi_init(nmi_handler)`.

RERI is mapped at `0x38010000` and implements eight error records. Protocol
tests that need a software-controlled source use record 5 so that hardware
producer behavior does not affect the result.

## Coverage

- Header fields: vendor ID, record count, version, and Valid Summary support.
- Record visibility: 64-bit Status and Address access, summary bits, and
  rejection of Status writes while the record is valid.
- Commands: `SRDP` sets `RDIP`; `SINV` clears `V` while preserving `RDIP` and
  the Control configuration.
- EID injection: preload a CE record, start the delay counter, and verify that
  `V`, `RDIP`, and the Address become visible after the delay expires.
- MMIO32 access: write each 32-bit half of the Address register for every
  record and verify that the other half is preserved.
- NMI delivery: inject a UEC record with `UECS=2` and `EID=1`, verify that the
  NMI handler runs, clear the source, and verify that execution resumes.

The default run performs 100 injection steps. Every group of four steps covers
one Record case followed by Commands, EID, and NMI, so the default run performs
25 NMI tests. The Record case rotates across all eight records.

## NMI Integration

The application registers an NMI handler with `_nmi_init()` and does not
override `__am_timervec` or maintain its own `mscratch` save area.

The shared AM trap path:

1. Saves the integer register context.
2. Detects an NMI entry with `mnstatus.NMIE`.
3. Exposes `mncause`, `mnepc`, and `mnstatus` as `scause`, `sepc`, and
   `sstatus` in `_Context`.
4. Calls the registered handler.
5. Returns with `mnret`.

The handler records the NMI cause, EPC, status, valid UEC source bitmap, and the
Status and Address of record 5. It then scans every record and, for each valid
record with UEC notification enabled, writes `SRDP | SINV` while preserving the
Control configuration. MMIO writes are ordered with `fence iorw, iorw`.

The main program waits for the handler count to advance instead of using `V` as
the completion condition, because `V` is still clear before delayed injection
takes effect. After the NMI returns, the test verifies:

- `mncause` is `(1ULL << 63) | 31`.
- The saved EPC is nonzero.
- `mnstatus.NMIE` was clear on entry.
- The expected UEC source record is set and its Status and Address are intact.
- The source record is invalidated while `RDIP` and Control are preserved.
- No duplicate NMI is delivered after a settle delay.
- Later UEC injections still enter the handler, confirming that `mnret` restored
  NMI delivery.

Hardware SRAM ECC detection, hardware producer routing, and PLIC external
interrupt handling are covered by other tests and are outside this application.

## Build

From the root of the `nexus-am` repository:

```sh
export AM_HOME="$PWD"
make -C apps/reri_test ARCH=riscv64-xs
```

The output image is:

```text
apps/reri_test/build/reri_test-riscv64-xs.bin
```

The default toolchain prefix is `riscv64-unknown-linux-gnu-`. Set
`LINUX_GNU_TOOLCHAIN=1` when using a system `riscv64-linux-gnu-` toolchain.

## Configuration

The number of injection steps and progress reporting interval can be changed at
compile time:

```sh
make -C apps/reri_test clean
make -C apps/reri_test \
  ARCH=riscv64-xs \
  RERI_INJECTIONS=32 \
  RERI_PROGRESS_STEP=8
```

`RERI_INJECTIONS` must be at least 4, and `RERI_PROGRESS_STEP` must be positive.
Thirty-two injection steps cover all records and execute eight NMI tests.
MMIO32 testing always covers all eight records regardless of the configured
injection count.

## Running

Run the generated image on a XiangShan emulator that implements the RERI layout
and Smrnni NMI support. When using `--no-diff`, the RTL and application
assertions determine the result.

A successful run reports every category as `PASS`, followed by:

```text
reri_test injections=100 failed=0
```

The injection count follows the build configuration. The default expected NMI
count is 25, and the run must finish with `HIT GOOD TRAP`.
