# DMA AXI system self-test

Runs on the complete XiangShan SoC with
`CONFIG=KunminghuV2WithoutLLCConfig ENABLE_DMA_SIM=1`. The simulator
fixture supplies AXI transactions through the external DMA port; CPU software
checks returned bytes and coherent loads. Real DCache/L2/DDR are used throughout.

```sh
make AM_HOME=/nfs/home/zhanglinjuan/OpenXiangShan/nexus-am ARCH=riscv64-xs
# In the XiangShan repository:
make emu CONFIG=KunminghuV2WithoutLLCConfig ENABLE_DMA_SIM=1
/path/to/XiangShan/build/emu --no-diff -i build/dma-system-riscv64-xs.bin
```

`ENABLE_DMA_SIM` defaults to `0`. When disabled, the AXI driver, control page,
fixture ports, PMA entry, and coverage counters are not generated. The real SoC
DMA datapath remains present, with its simulation input tied idle. Switching the
make variable regenerates SimTop RTL automatically. `DMASystemTestConfig` is kept
as a hardware-configuration alias and also requires `ENABLE_DMA_SIM=1`.

The test requires the optional simulation-only control page at `0x40080000`.
It is not a real DMA-device model. Data buffers occupy `0x81000000..0x81ffffff`;
cold-write pressure uses `0x82000000 + n * 0x10000` (n < 128), and CPU
pressure stores use `0x90000000 + n * 0x10000` (n < 128).
Set-conflict loads for eviction use `0x84000000 + n * 0x100000` (n < 64).
Keep workload code/stack and other applications outside those ranges.

Eight checks: cold DDR read; CPU dirty-line to DMA read; DMA write to CPU load;
DMA write to DMA read; sparse/contiguous/zero strobes; cold write/replay/retry;
CPU Sbuffer and DMA concurrent writes; actual L1/L2 eviction and DDR refill.
Each checks data independently in software and requires its target hardware
coverage counters to be nonzero. The test prints eight `[DMA PASS]` lines and
`DMA SYSTEM TEST: ALL 8 CASES PASSED`, then exits through the AM success trap.

`--no-diff` preserves DUT trap detection. NEMU/GoldenMem are not used. A command
that fails to complete within two million DUT cycles reports a software failure;
there is no external wall-clock timeout.

Control registers (aligned 64-bit accesses):

| Offset | Meaning |
| --- | --- |
| 0x00 | fixture signature `DMATEST1` |
| 0x08 | start: 0=read, 1=write |
| 0x10 | physical address |
| 0x18 | 32-bit WSTRB |
| 0x20 | SIZE[2:0], LEN[15:8], apply R/B stalls[16] |
| 0x28 | AXI ID |
| 0x30 | busy[0], done[1], errors[23:8] |
| 0x38 | transaction repetition count |
| 0x40 | address stride between transactions |
| 0x48 | completed transaction count |
| 0x80/0x90 | watched physical base/mask |
| 0x88 | clear coverage counters |
| 0x100..0x118 | four 64-bit words of WDATA |
| 0x200..0x3f8 | collected RDATA, up to 16 beats |
| 0x400..0x458 | twelve coverage counters |

WDATA repeats on each beat. Counters, in order: DMA TL Get; DCache Probe;
ProbeAckData; DMA DCache grant; DMA replay; DMA hit; Sbuffer grant;
cycles with a selected valid DMA request and Sbuffer valid; ReleaseData; GrantData; watched DDR AR; watched
DDR AW. Counts may be per beat, not per line. The GrantData counter observes all
DCache refills; address-specific tests use the address-bearing bus counters.

A cold miss can succeed after a refill without a replay response. Case 6 adds
real CPU same-set store pressure, requires a nonzero DMA replay counter, and
checks all 128 complete cache lines after successful retries.
