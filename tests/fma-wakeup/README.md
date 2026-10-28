# AM FMA wakeup and recovery validation

This application checks fused arithmetic and supplies stable assembly regions for
GEM5 pipeline traces. It uses normal nexus-am startup and halt, the RV64GC ISA,
integer IEEE bit-pattern oracles, and caller-saved registers in leaf assembly.
The default image runs every case and prints exactly one `FMA_WAKEUP_PASS` line
only after all results and exception flags pass. A failure prints its case,
index, actual/expected bits and flags, and returns nonzero.

Build with the system GNU RISC-V toolchain:

```sh
export AM_HOME=/path/to/nexus-am
make -C "$AM_HOME/tests/fma-wakeup" \
  ARCH=riscv64-xs MARCH=rv64gc LINUX_GNU_TOOLCHAIN=1
```

The build produces `build/fma-wakeup-riscv64-xs.elf`, `.bin`, and the `.txt`
disassembly under `tests/fma-wakeup`. Run the complete GEM5/NEMU validation
from the GEM5 repository root:

```sh
python3 util/xs_scripts/test_am_fma.py --am-home "$AM_HOME" \
  --outdir /tmp/am-fma-validation
```

The runner contains no adaptive-policy tests. `mainargs=functional`,
`mainargs=wakeup`, or `mainargs=recovery` can select a subset; empty arguments and `mainargs=all` run all cases.
Rebuild when changing `mainargs`, since it is embedded by AM.

## Arithmetic checks

For both binary32 and binary64, all four fused forms independently compute
`2 * 3 + 1 = 7`, `2 * 3 - 1 = 5`, `-(2 * 3) + 1 = -5`, and
`-(2 * 3) - 1 = -7`. Each width also checks destination overlap with each of
rs1/rs2/rs3, a separate FP producer for each source, and one FP producer supplying
all three operands (`2 * 2 + 2 = 6`). Full 64-bit stores verify binary32 NaN
boxing as well as the low 32 result bits.

The fused round-once vectors are `(1 + 2^-23) * (1 - 2^-23) - 1 = -2^-46`
for binary32 (low bits `0xa8800000`) and
`(1 + 2^-52) * (1 - 2^-52) - 1 = -2^-104` for binary64
(`0xb970000000000000`). These results are exact, so every functional case
expects zero accrued flags. No C floating-point calculation is used.

## Trace regions

Every region below exports `<name>_begin` and `<name>_end`. End labels are
exclusive; constants and flag initialization precede the timing regions.
All timing operands and results change exactly, avoiding rounding-dependent
or constant-valued dependency chains.

| Region | Instructions / exact final result | Expected minimum select distance |
| --- | --- | --- |
| `add_chain` | 64 dependent additions of 1, from 1 to 65 | ADD 2 |
| `mul_chain` | 64 dependent multiplies by 2, from 1 to 2^64 | MUL 3 |
| `fma_multiplicand_chain` | 64 `x * 1 + 1` operations, rs1 dependency, final 65 | FMA multiplicand 4 |
| `fma_rs2_chain` | 64 `1 * x + 1` operations, rs2 dependency, final 65 | FMA multiplicand 4 |
| `fma_accumulator_chain` | 64 `1 * 1 + x` operations, final 65 | FMA addend 2 |
| `mul_to_fma_acc` | 32 pairs `x *= 2; x = 1 * 1 + x`, final 2^33 - 1 | MUL to addend 1 |
| `duplicate_source_wakeup` | 24 MUL producers double x; FMA uses x in all sources, final 2^48 + 2^24 | MUL to duplicate source 3 |

`late_arrival` places 16 explicit nops between `2 * 3` and the FMA consumer
`6 * 2 + 1 = 13`. `fma_to_store` stores 7 from an FMA;
`fma_to_convert` converts 7 to a signed integer with RTZ and verifies integer
bits 7. Their corresponding begin/end symbols are exported.

## Load recovery, pressure, and branch squash

The hot addend and multiplicand arrays contain 256 consecutive binary64 values
1 through 256 and are explicitly prewarmed before their ROIs. Every cold array
has 256 nodes at 4096-byte strides. Input values are initialized in the image's
read-only data, and cold arrays are untouched before their corresponding
regions. Each experiment uses its own array. These are cold on first access
from the application's perspective; actual cache misses and recovery events
must still be confirmed in the simulator trace.

`load_hot_addend`, `load_cold_addend` compute `6 + i`;
`load_hot_multiplicand`, `load_cold_multiplicand` compute `3 * i + 2`,
for `i = 1..256`. The exact target FMA instructions have global labels
`load_cold_addend_fma` and `load_cold_multiplicand_fma`.

`fma_cascade` runs `fld -> FMA addend -> FMA rs1` for each cold input.
`cascade_fma_one` computes `i + 6`; `cascade_fma_two` computes
`2 * (i + 6) + 1 = 2 * i + 13`. Both labels point directly to their FMAs.

`fma_pressure` places a target load/FMA addend dependency beside eight
independent FMA recurrence chains. All 256 target outputs and all eight final
recurrences (257) are checked. Memory ROIs store every output and check the
aggregate accrued flags after the region; CSR reads are kept outside these
ROIs to avoid serializing speculative loads and FMAs.

`branch_squash` trains a static forward branch to fall through 255 times with
a valid `1 * 0 + 0` FMA. Its final cold node supplies a taken condition and
infinity: the fall-through instruction at `wrongpath_fma` would compute
`inf * 0 + 0` and set NV if it committed. Four dependent integer multiplies
between the cold condition load and branch delay resolution. The wrong-path
destination starts at +0; valid training FMAs preserve +0. Every committed path
computes 7, adds the preserved destination, and checks result bits and zero
flags. A leaked wrong-path NaN therefore also fails the destination oracle.
Execution/squashing of this invalid
wrong-path FMA is a trace coverage requirement; arithmetic success by itself
does not prove that the predictor speculated down this path.

Pipeline acceptance must check trace evidence for actual FU admission,
cancellation and a newer successful attempt, along with the applicable timing
bounds. This app deliberately contains no adaptive-policy tests or simulator
pseudo-instructions.
