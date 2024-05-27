# Counter Test

This program provides a self-test for the counter CSRs, testing the following features:
- reading and writing `mcycle`, `mtime`(no write), `minstret`, `mhpmcounter3` - `mhpmcounter31`.
- reading `cycle`, `time`, `instret`, `hpmcounter3` - `hpmcounter31`, and corresponding permission check according to `mcounteren`, `hcounteren`, and `scounteren`.
- Functionality of `mcountinhibit`.

This program provides a simple framework for privilege mode switching and expected exception capture, which could be applied to other test programs.
