# XiangShan#6720

Some of the pc value are sign-extended from 50 bit to 51 bit, and the ITLB is modified to use `guardedPc[49]` to distinguish between a sign-extended `guardedPc[50]` and a overflowed one. However, CSR is not modified, so it can report a wrong xepc/xtval value while triggering an exception in Sv48x4 mode with `pc[49] = 1`.

## Related sources

- tests/fetch-canonical-check-test
- XiangShan#6751: port for kunminghu-v3
- XiangShan#6272: canonical pc check fix, which passes tests/fetch-canonical-check-test but introduced this issue
