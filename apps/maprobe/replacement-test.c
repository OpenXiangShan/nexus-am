#include "maprobe.h"

void generate_replacement_test_matrix()
{
#define REPLACEMENT_TEST_MAX_WAY 17 // up to 16 way
#define REPLACEMENT_TEST_ITER 5 // 1 warmup + 4 test
    assert(REPLACEMENT_TEST_ITER >= 2);
    DEFINE_FLOAT_RESULT_MATRIX(replacement_test,num_way_accessed,REPLACEMENT_TEST_MAX_WAY,iter,REPLACEMENT_TEST_ITER);
    FOR(x,REPLACEMENT_TEST_ITER) { replacement_test_column_array[x] = x; }
    for (int i = 0; i < REPLACEMENT_TEST_MAX_WAY; i++) {
        replacement_test_row_array[i] = i+1;
        int warm_up_iter = 64;
        int test_iter = i < 4 ? 256 : 64;
        replacement_test_result_array[i][0] = test_linear_access_latency_simple(_PERF_ADDR_STRIDE_L1_SAME_SET*(i+1),_PERF_ADDR_STRIDE_L1_SAME_SET,warm_up_iter,0); //warmup
        for(int j = 1; j < REPLACEMENT_TEST_ITER; j++) {
            replacement_test_result_array[i][j] = test_linear_access_latency_simple(_PERF_ADDR_STRIDE_L1_SAME_SET*(i+1),_PERF_ADDR_STRIDE_L1_SAME_SET,test_iter,0); //test
        }
    }
    print_float_result_matrix(&replacement_test_matrix_meta);
}