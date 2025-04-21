# Branch Target Prediction Test Suite (Pure C Implementation)

This test suite is designed to test the target prediction capabilities of a branch predictor using standard C code without inline assembly. It includes tests for different types of branch instructions and their target prediction.

## Test Files

The test suite includes the following test files:

1. **indirect_branch.c** - Tests indirect branch target prediction using function pointers.
2. **return_branch.c** - Tests return instruction target prediction with calls from different sites.
3. **call_branch.c** - Tests call target prediction with patterns that call different functions.
4. **jump_branch.c** - Tests unconditional jump target prediction using goto statements.
5. **combined_target_test.c** - Comprehensive test that exercises all branch types in one program.

## Test Patterns

Each test creates predictable patterns of branch targets to exercise the branch target predictor:

- **Indirect Branch Test**: Uses function pointers to create indirect branches to different target functions in a pattern.
- **Return Branch Test**: Calls the same function from different call sites to test return address prediction.
- **Call Branch Test**: Calls different functions in a repeating pattern to test call target prediction.
- **Jump Branch Test**: Uses goto statements to create unconditional jumps to different targets in a pattern.
- **Combined Test**: Alternates between all types of branches to test comprehensive prediction.

## Implementation Notes

1. All tests use pure C code without inline assembly for better portability.
2. Each test creates repeating patterns to help train the branch predictor.
3. The `__attribute__((noinline))` attribute is used to prevent the compiler from optimizing away function calls.
4. No I/O is used to avoid interfering with timing measurements.
5. Each test runs 1000 iterations to give the branch predictor sufficient training data.

## Running the Tests

To compile and run a specific test:

```
cd tests/brtargettest2
make ${TEST_NAME}
```

For example:
```
make indirect_branch
```

## Expected Behavior

These tests don't produce any output as they're intended for performance measurement. A good branch target predictor should be able to learn the patterns and predict correctly after several iterations. 