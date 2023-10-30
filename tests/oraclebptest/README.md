# OracleBP Test

## Purpose

A fully functional OracleBP should predict well even after some unpredictable events like interrupt and MMIO accessing happen. This test wants to verify if the oracle predictor still works after a MMIO access that affects the control flow.

## Verification

Change final loop iteration count to see if there are too many mispredictions.