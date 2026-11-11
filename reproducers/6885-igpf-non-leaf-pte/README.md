# XiangShan#6885

When a guest-page fault is caused by an implicit VS-stage page-table access, the TLB sets `isForVSnonLeafPTE` to indicate that the fault is for a VS-stage PTE read (non-leaf or invalid PTE), rather than for the original instruction fetch. In this case `mtval2` (`htval` when the fault is delegated to HS-mode) must contain the GPA of the page-table entry access, not the GPA of the fetch block.

#6885 reports that XiangShan's instruction-fetch path does not preserve this distinction. IPrefetch receives `gpaddr` from the iTLB and stores it in `GPAMem`; when the instruction traps, the backend reads it back from `GPAMem` and unconditionally applies the fetch-block offset calculation. The offset is therefore also applied to the GPA of the PTE access, which does not match the expected behavior.

This test builds page tables in which the GPA of an intermediate VS-stage PTE is left unmapped by G-stage, and jumps to a target with a nonzero page offset. The implicit PTE read raises the guest-page fault, and the test checks the reported `mtval2`. The target VA is `0x8020100c`; its VS L0 PTE read is at GPA `0x8003f008`, so the expected value is `mtval2 = 0x2000fc02`.

## Related sources

- [#6885](https://github.com/OpenXiangShan/XiangShan/issues/6885): original issue for the incorrect PTE GPA in the instruction-fetch guest-page fault.
- [#6911](https://github.com/OpenXiangShan/XiangShan/pull/6911): v2 fix; selects the raw GPA in ICache and `GPAMem`.
- [#6915](https://github.com/OpenXiangShan/XiangShan/pull/6915): v3 fix; selects the raw GPA in `GPAMem`. ICache does not need to be changed in v3 because v3 does not allow cross-page fetches, so there is no offset applied there.
- [#6870](https://github.com/OpenXiangShan/XiangShan/issues/6870): separate `getGPAddr` low-12-bit issue.
