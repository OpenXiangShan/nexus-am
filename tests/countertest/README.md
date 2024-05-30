# Counter Test

This program provides a self-test for the counter CSRs, testing the following features:
- reading and writing `mcycle`, `mtime`(no write), `minstret`, `mhpmcounter3` - `mhpmcounter31`.
- reading `cycle`, `time`, `instret`, `hpmcounter3` - `hpmcounter31`, and corresponding permission check according to `mcounteren`, `hcounteren`, and `scounteren`.
- Functionality of `mcountinhibit`.

This program provides a simple framework for privilege mode switching and expected exception capture, which could be applied to other test programs.

## How-to
In `tests/countertest`, run
```
make ARCH=riscv64-xs
```
You will get `countertest-riscv64-xs.bin` in `tests/countertest/build`.

This program would `HIT GOOD TRAP` if there were no fatal error.
