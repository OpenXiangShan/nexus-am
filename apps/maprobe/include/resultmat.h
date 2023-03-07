#ifndef PROBE_RESULT_MATRIX_H
#define PROBE_RESULT_MATRIX_H

#include <klib.h>

struct result_matrix_meta {
    char* name;
    char* row_name;
    char* column_name;
    int row_size;
    int column_size;
    void* result_array;
    void* column_array;
    void* row_array;
};

void print_float_result_matrix(struct result_matrix_meta* meta);
void matrix_print_example();

#define FOR(v,end) for (int v = 0; v < end; v++)
#define CONCAT(a,b) a##b
#define TOSTR(a) #a
#define DEFINE_FLOAT_RESULT_MATRIX(matrix_name, rowname, rowsize, columnname, columnsize) \
    struct result_matrix_meta CONCAT(matrix_name,_matrix_meta); \
    float CONCAT(matrix_name,_result_array)[rowsize][columnsize] = {0}; \
    int CONCAT(matrix_name,_column_array)[columnsize] = {0}; \
    int CONCAT(matrix_name,_row_array)[rowsize] = {0}; \
    CONCAT(matrix_name,_matrix_meta).name = TOSTR(matrix_name); \
    CONCAT(matrix_name,_matrix_meta).column_name = TOSTR(columnname); \
    CONCAT(matrix_name,_matrix_meta).row_name = TOSTR(rowname); \
    CONCAT(matrix_name,_matrix_meta).column_size = columnsize; \
    CONCAT(matrix_name,_matrix_meta).row_size = rowsize; \
    CONCAT(matrix_name,_matrix_meta).result_array = CONCAT(matrix_name,_result_array); \
    CONCAT(matrix_name,_matrix_meta).column_array = CONCAT(matrix_name,_column_array); \
    CONCAT(matrix_name,_matrix_meta).row_array = CONCAT(matrix_name,_row_array);

#endif