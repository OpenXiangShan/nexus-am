#!/usr/bin/env python3

# Dictionary of branch predictor test paths
branch_tests = {
    "always_taken": "/nfs/home/xuboran/tools/nexus-am/tests/frontendtest/cond_br_test/build/always_taken_test-riscv64-xs.bin",
    "never_taken": "/nfs/home/xuboran/tools/nexus-am/tests/frontendtest/cond_br_test/build/never_taken_test-riscv64-xs.bin",
    "alternating": "/nfs/home/xuboran/tools/nexus-am/tests/frontendtest/cond_br_test/build/alternating_test-riscv64-xs.bin",
    "two_bit_pattern": "/nfs/home/xuboran/tools/nexus-am/tests/frontendtest/cond_br_test/build/two_bit_pattern_test-riscv64-xs.bin",
    "three_bit_pattern": "/nfs/home/xuboran/tools/nexus-am/tests/frontendtest/cond_br_test/build/three_bit_pattern_test-riscv64-xs.bin",
    "prime_based_pattern": "/nfs/home/xuboran/tools/nexus-am/tests/frontendtest/cond_br_test/build/prime_based_pattern_test-riscv64-xs.bin",
    "switching_pattern": "/nfs/home/xuboran/tools/nexus-am/tests/frontendtest/cond_br_test/build/switching_pattern_test-riscv64-xs.bin",
    "nested_branches": "/nfs/home/xuboran/tools/nexus-am/tests/frontendtest/cond_br_test/build/nested_branches_test-riscv64-xs.bin",
    "early_exits": "/nfs/home/xuboran/tools/nexus-am/tests/frontendtest/cond_br_test/build/early_exits_test-riscv64-xs.bin",
    "rare_branches": "/nfs/home/xuboran/tools/nexus-am/tests/frontendtest/cond_br_test/build/rare_branches_test-riscv64-xs.bin",
    "gradual_transition": "/nfs/home/xuboran/tools/nexus-am/tests/frontendtest/cond_br_test/build/gradual_transition_test-riscv64-xs.bin",
    "aliasing_pattern": "/nfs/home/xuboran/tools/nexus-am/tests/frontendtest/cond_br_test/build/aliasing_pattern_test-riscv64-xs.bin",
    "all_patterns": "/nfs/home/xuboran/tools/nexus-am/tests/frontendtest/cond_br_test/build/all_patterns_test-riscv64-xs.bin"
}

# You can also access the dictionary in JSON format
if __name__ == "__main__":
    import json
    print(json.dumps(branch_tests, indent=4)) 