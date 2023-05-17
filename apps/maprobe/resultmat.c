#include "resultmat.h"

void print_float_result_matrix(struct result_matrix_meta* meta)
{
    assert(meta);
    printf("---------- %s matrix start ----------\n", meta->name);
    printf("%s (row) \\ %s (column)\n", meta->row_name, meta->column_name);
    if (meta->column_array) {
        if (meta->row_array) {
            printf("\\  ,   \t");
        }
        for (int c = 0; c < meta->column_size; c++) {
            printf("      %d,\t", *((int*)meta->column_array + c));
        }
        printf("\n");
    }
    for (int r = 0; r < meta->row_size; r++) {
        if (meta->row_array) {
            printf("%3d,\t", *((int*)meta->row_array + r));
        }
        for (int c = 0; c < meta->column_size; c++) {
            printf("%f,\t", *((float*)meta->result_array + r * meta->column_size + c));
        }
        printf("\n");
    }
    printf("---------- %s matrix end ----------\n");
}

void matrix_print_example()
{
    DEFINE_FLOAT_RESULT_MATRIX(test,testrow,5,testcol,10);
    // ({ 
    //     struct result_matrix_meta test_matrix_meta;
    //     float test_result_array[5][10] = {0};
    //     int test_column_array[10] = {0};
    //     int testrow_array[5] = {0};
    //     test_matrix_meta.name = "test";
    //     test_matrix_meta.column_name = "testcol";
    //     test_matrix_meta.row_name = "testrow";
    //     test_matrix_meta.column_size = 10;
    //     test_matrix_meta.row_size = 5;
    //     test_matrix_meta.result_array = test_result_array;
    //     test_matrix_meta.column_array = test_column_array;
    //     test_matrix_meta.row_array = test_row_array;
    // })

    FOR(x,5) { test_row_array[x] = x; }
    FOR(x,10) { test_column_array[x] = x; }
    FOR(x,5) {
        FOR(y,10) {
            test_result_array[x][y] = rand();
        }
    }
    print_float_result_matrix(&test_matrix_meta);
}