#include <amdev.h>
#include <klib.h>
#include <stdint.h>
#include <unistd.h>

/**
 * @brief CPU implementation of matrix multiplication (reference implementation)
 *
 * Basic matrix multiplication using triple loops: C = A × B
 * Time complexity: O(M×N×K)
 * Memory layout: Row-major order
 *
 * @param input0 Input matrix A, dimensions M×K, row-major storage
 * @param input1 Input matrix B, dimensions K×N, row-major storage
 * @param dst Output matrix C, dimensions M×N, row-major storage
 * @param M Number of rows in matrix A and C
 * @param K Number of columns in matrix A, rows in matrix B
 * @param N Number of columns in matrix B and C
 */
void matmul_cpu(float *input0, float *input1, float *dst, int M, int K, int N) {
  for (int i = 0; i < M; i++) {   // Iterate over rows of output matrix
    for (int j = 0; j < N; j++) { // Iterate over columns of output matrix
      float sum = 0.0f;           // Accumulator, initialize to 0

      // Compute dot product: C[i][j] = Σ_{k=0}^{K-1} A[i][k] × B[k][j]
      for (int k = 0; k < K; k++) {
        sum += input0[i * K + k] * input1[k * N + j];
      }
      dst[i * N + j] = sum; // Store result
    }
  }
}

/**
 * @brief Configure TCSR0 register group for Tensor accelerator
 *
 * TCSR (Tensor Control and Status Register) controls the memory access pattern
 * for tensor operations. This function configures TCSR0 for matrix A (left
 * operand).
 *
 * @param m Dimension 0 size (corresponds to M in matrix multiplication)
 * @param n Dimension 1 size (corresponds to K in matrix multiplication)
 *
 * @note TCSR register layout (each TCSR controls one dimension):
 *       - dim0: Dimension 0 size (tcsr_id = 0)
 *       - stride0: Dimension 0 stride (tcsr_id = 1)
 *       - dim1: Dimension 1 size (tcsr_id = 2)
 *       - stride1: Dimension 1 stride (tcsr_id = 3)
 *       - dim2: Dimension 2 size (fixed to 1, tcsr_id = 4)
 *       - stride2: Dimension 2 stride (tcsr_id = 5)
 *       - dim3: Dimension 3 size (fixed to 1, tcsr_id = 6)
 *       - stride3: Dimension 3 stride (tcsr_id = 7)
 * @note Instruction format (tx.tcsr.xchg.nmask):
 *       63-56: 10000000 (opcode)
 *       41-32: tcsr_id  (register ID)
 *       31-27: rs3      (reserved)
 *       19-15: rs1      (source register, holds value to write)
 *       11-7:  rd       (destination register, usually 0)
 *       6-0:  0111111   (function code)
 */
static inline void config_tcsr0(int m, int n) {
  // dim0: Set dimension 0 size = m (tcsr_id = 0)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(m) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x00, 0x00, 0x00, 0x80");

  // stride0: Set dimension 0 stride = n (tcsr_id = 1)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(n) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x01, 0x00, 0x00, 0x80");

  // dim1: Set dimension 1 size = n (tcsr_id = 2)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(n) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x02, 0x00, 0x00, 0x80");

  // stride1: Set dimension 1 stride = 1 (tcsr_id = 3)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(1) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x03, 0x00, 0x00, 0x80");

  // dim2: Set dimension 2 size = 1 (tcsr_id = 4)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(1) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x04, 0x00, 0x00, 0x80");

  // stride2: Set dimension 2 stride = m*n (tcsr_id = 5)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(m * n) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x05, 0x00, 0x00, 0x80");

  // dim3: Set dimension 3 size = 1 (tcsr_id = 6)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(1) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x06, 0x00, 0x00, 0x80");

  // stride3: Set dimension 3 stride = m*n (tcsr_id = 7)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(m * n) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x07, 0x00, 0x00, 0x80");
}

/**
 * @brief Configure TCSR1 register group for Tensor accelerator
 *
 * Configures TCSR1 for matrix B (right operand) in matrix multiplication.
 * Similar to config_tcsr0 but uses different register IDs (8-15).
 *
 * @param m Dimension 0 size (corresponds to K in matrix multiplication)
 * @param n Dimension 1 size (corresponds to N in matrix multiplication)
 */
static inline void config_tcsr1(int m, int n) {
  // dim0: Set dimension 0 size = m (tcsr_id = 8)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(m) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x08, 0x00, 0x00, 0x80");

  // stride0: Set dimension 0 stride = n (tcsr_id = 9)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(n) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x09, 0x00, 0x00, 0x80");

  // dim1: Set dimension 1 size = n (tcsr_id = 10)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(n) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x0a, 0x00, 0x00, 0x80");

  // stride1: Set dimension 1 stride = 1 (tcsr_id = 11)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(1) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x0b, 0x00, 0x00, 0x80");

  // dim2: Set dimension 2 size = 1 (tcsr_id = 12)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(1) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x0c, 0x00, 0x00, 0x80");

  // stride2: Set dimension 2 stride = m*n (tcsr_id = 13)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(m * n) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x0d, 0x00, 0x00, 0x80");

  // dim3: Set dimension 3 size = 1 (tcsr_id = 14)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(1) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x0e, 0x00, 0x00, 0x80");

  // stride3: Set dimension 3 stride = m*n (tcsr_id = 15)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(m * n) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x0f, 0x00, 0x00, 0x80");
}

/**
 * @brief Configure TCSR2 register group for Tensor accelerator
 *
 * Configures TCSR2 for matrix C (output) in matrix multiplication.
 * Uses register IDs 16-23.
 *
 * @param m Dimension 0 size (corresponds to M in matrix multiplication)
 * @param n Dimension 1 size (corresponds to N in matrix multiplication)
 */
static inline void config_tcsr2(int m, int n) {
  // dim0: Set dimension 0 size = m (tcsr_id = 16)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(m) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x10, 0x00, 0x00, 0x80");

  // stride0: Set dimension 0 stride = n (tcsr_id = 17)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(n) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x11, 0x00, 0x00, 0x80");

  // dim1: Set dimension 1 size = n (tcsr_id = 18)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(n) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x12, 0x00, 0x00, 0x80");

  // stride1: Set dimension 1 stride = 1 (tcsr_id = 19)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(1) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x13, 0x00, 0x00, 0x80");

  // dim2: Set dimension 2 size = 1 (tcsr_id = 20)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(1) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x14, 0x00, 0x00, 0x80");

  // stride2: Set dimension 2 stride = m*n (tcsr_id = 21)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(m * n) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x15, 0x00, 0x00, 0x80");

  // dim3: Set dimension 3 size = 1 (tcsr_id = 22)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(1) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x16, 0x00, 0x00, 0x80");

  // stride3: Set dimension 3 stride = m*n (tcsr_id = 23)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(m * n) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x17, 0x00, 0x00, 0x80");
}

/**
 * @brief Synchronized load from main memory to URAM (Unified RAM)
 *
 * Transfers data from system memory to accelerator's unified RAM with
 * specified tensor dimensions and strides. This operation is synchronous
 * (blocks until completion).
 *
 * @param dst Destination pointer in URAM
 * @param src Source pointer in system memory
 * @param dim0 Dimension 0 size
 * @param dim1 Dimension 1 size
 * @param dim2 Dimension 2 size
 * @param dim3 Dimension 3 size
 * @param stride0 Stride for dimension 0 (in elements)
 * @param stride1 Stride for dimension 1 (in elements)
 * @param stride2 Stride for dimension 2 (in elements)
 * @param stride3 Stride for dimension 3 (in elements)
 *
 * @note This function configures both source and destination tensor descriptors
 * @note Uses custom RISC-V extension instructions for tensor operations
 */
void __tx_sync_uram_load(float *dst, float *src, int dim0, int dim1, int dim2,
                         int dim3, int stride0, int stride1, int stride2,
                         int stride3) {
  // Configure source tensor dimensions and strides (TCSR 0-7)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(dim0) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x00, 0x00, 0x00, 0x80");

  __asm__ volatile("mv x5, %0\n\t" : : "r"(dim1) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x02, 0x00, 0x00, 0x80");

  __asm__ volatile("mv x5, %0\n\t" : : "r"(dim2) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x04, 0x00, 0x00, 0x80");

  __asm__ volatile("mv x5, %0\n\t" : : "r"(dim3) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x06, 0x00, 0x00, 0x80");

  // Configure source tensor strides
  __asm__ volatile("mv x5, %0\n\t" : : "r"(stride0) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x01, 0x00, 0x00, 0x80");
  __asm__ volatile("mv x5, %0\n\t" : : "r"(stride1) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x03, 0x00, 0x00, 0x80");
  __asm__ volatile("mv x5, %0\n\t" : : "r"(stride2) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x05, 0x00, 0x00, 0x80");
  __asm__ volatile("mv x5, %0\n\t" : : "r"(stride3) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x07, 0x00, 0x00, 0x80");

  // Configure destination tensor dimensions and strides (TCSR 8-15)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(dim0) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x08, 0x00, 0x00, 0x80");
  __asm__ volatile("mv x5, %0\n\t" : : "r"(dim1) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x0a, 0x00, 0x00, 0x80");
  __asm__ volatile("mv x5, %0\n\t" : : "r"(dim2) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x0c, 0x00, 0x00, 0x80");
  __asm__ volatile("mv x5, %0\n\t" : : "r"(dim3) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x0e, 0x00, 0x00, 0x80");
  __asm__ volatile("mv x5, %0\n\t" : : "r"(stride0) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x09, 0x00, 0x00, 0x80");
  __asm__ volatile("mv x5, %0\n\t" : : "r"(stride1) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x0b, 0x00, 0x00, 0x80");
  __asm__ volatile("mv x5, %0\n\t" : : "r"(stride2) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x0d, 0x00, 0x00, 0x80");
  __asm__ volatile("mv x5, %0\n\t" : : "r"(stride3) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x0f, 0x00, 0x00, 0x80");

  // Execute synchronized URAM load instruction
  // tx.sync.uram.load: Load from system memory to URAM
  __asm__ volatile("mv x6, %0\n\t" : : "r"(dst) : "x5");
  __asm__ volatile("mv x5, %0\n\t" : : "r"(src) : "x6");
  asm volatile(".byte 0x3f, 0xa0, 0x02, 0x30, 0x40, 0x00, 0x00, 0x8a");
}

/**
 * @brief Synchronized store from URAM to main memory
 *
 * Transfers data from accelerator's unified RAM to system memory with
 * specified tensor dimensions and strides. This operation is synchronous.
 *
 * @param dst Destination pointer in system memory
 * @param src Source pointer in URAM
 * @param dim0 Dimension 0 size
 * @param dim1 Dimension 1 size
 * @param dim2 Dimension 2 size
 * @param dim3 Dimension 3 size
 * @param stride0 Stride for dimension 0 (in elements)
 * @param stride1 Stride for dimension 1 (in elements)
 * @param stride2 Stride for dimension 2 (in elements)
 * @param stride3 Stride for dimension 3 (in elements)
 *
 * @note Similar to __tx_sync_uram_load but in opposite direction
 */
void __tx_sync_uram_store(float *dst, float *src, int dim0, int dim1, int dim2,
                          int dim3, int stride0, int stride1, int stride2,
                          int stride3) {
  // Configure tensor dimensions and strides (TCSR 0-7)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(dim0) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x00, 0x00, 0x00, 0x80");

  __asm__ volatile("mv x5, %0\n\t" : : "r"(dim1) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x02, 0x00, 0x00, 0x80");

  __asm__ volatile("mv x5, %0\n\t" : : "r"(dim2) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x04, 0x00, 0x00, 0x80");

  __asm__ volatile("mv x5, %0\n\t" : : "r"(dim3) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x06, 0x00, 0x00, 0x80");

  // Configure tensor strides
  __asm__ volatile("mv x5, %0\n\t" : : "r"(stride0) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x01, 0x00, 0x00, 0x80");
  __asm__ volatile("mv x5, %0\n\t" : : "r"(stride1) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x03, 0x00, 0x00, 0x80");
  __asm__ volatile("mv x5, %0\n\t" : : "r"(stride2) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x05, 0x00, 0x00, 0x80");
  __asm__ volatile("mv x5, %0\n\t" : : "r"(stride3) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x07, 0x00, 0x00, 0x80");

  // Configure destination tensor (TCSR 8-15)
  __asm__ volatile("mv x5, %0\n\t" : : "r"(dim0) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x08, 0x00, 0x00, 0x80");
  __asm__ volatile("mv x5, %0\n\t" : : "r"(dim1) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x0a, 0x00, 0x00, 0x80");
  __asm__ volatile("mv x5, %0\n\t" : : "r"(dim2) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x0c, 0x00, 0x00, 0x80");
  __asm__ volatile("mv x5, %0\n\t" : : "r"(dim3) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x0e, 0x00, 0x00, 0x80");
  __asm__ volatile("mv x5, %0\n\t" : : "r"(stride0) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x09, 0x00, 0x00, 0x80");
  __asm__ volatile("mv x5, %0\n\t" : : "r"(stride1) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x0b, 0x00, 0x00, 0x80");
  __asm__ volatile("mv x5, %0\n\t" : : "r"(stride2) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x0d, 0x00, 0x00, 0x80");
  __asm__ volatile("mv x5, %0\n\t" : : "r"(stride3) : "x5");
  asm volatile(".byte 0x3f, 0x80, 0x02, 0x00, 0x0f, 0x00, 0x00, 0x80");

  // Execute synchronized URAM store instruction
  // tx.sync.uram.store: Store from URAM to system memory
  __asm__ volatile("mv x6, %0\n\t" : : "r"(dst) : "x5");
  __asm__ volatile("mv x5, %0\n\t" : : "r"(src) : "x6");
  asm volatile(".byte 0x3f, 0xa0, 0x02, 0x30, 0x40, 0x00, 0x00, 0x89");
}

/**
 * @brief Test matrix multiplication using Tensor accelerator
 *
 * This test function:
 * 1. Allocates memory for matrices A (M×K), B (K×N), and C (M×N)
 * 2. Initializes matrices with test data
 * 3. Transfers data to URAM using synchronized load
 * 4. Configures TCSR registers for tensor dimensions
 * 5. Executes matrix multiplication using tensor accelerator
 * 6. Transfers results back to system memory
 * 7. Compares results with CPU implementation
 * 8. Reports pass/fail status
 *
 * @return int 0 on success, non-zero on failure
 *
 * @note Test dimensions: M=16, K=32, N=64 (small test case)
 * @note Memory cleanup performed at the end
 */
int test_matmul() {
  int M = 16; // Rows in matrix A and C
  int K = 32; // Columns in A, rows in B
  int N = 64; // Columns in B and C

  // Allocate and initialize matrix A (M×K)
  float *src0 = (float *)malloc(sizeof(float) * M * K);
  if (!src0) {
    printf("Error: cannot alloc memory\n");
    return 0;
  }
  int i = 0;
  for (; i < M * K; i++) {
    src0[i] = (float)(i * 5); // Simple linear pattern for testing
  }

  // Allocate URAM buffer for matrix A
  float *uram_src0 = (float *)malloc(sizeof(float) * M * K);
  if (!uram_src0) {
    printf("Error: cannot alloc memory\n");
    return 0;
  }

  // Allocate and initialize matrix B (K×N)
  float *src1 = (float *)malloc(sizeof(float) * K * N);
  if (!src1) {
    printf("Error: cannot alloc memory\n");
    return 0;
  }
  for (i = 0; i < K * N; i++) {
    src1[i] = (float)(i * 3); // Different pattern for verification
  }

  // Allocate URAM buffer for matrix B
  float *uram_src1 = (float *)malloc(sizeof(float) * K * N);
  if (!uram_src1) {
    printf("Error: cannot alloc memory\n");
    return 0;
  }

  // Allocate output buffers for accelerator and CPU
  float *dst0 = (float *)malloc(sizeof(float) * M * N); // Accelerator output
  if (!dst0) {
    printf("Error: cannot alloc memory\n");
    return 0;
  }

  float *uram_dst0 =
      (float *)malloc(sizeof(float) * M * N); // URAM output buffer
  if (!uram_dst0) {
    printf("Error: cannot alloc memory\n");
    return 0;
  }

  float *dst1 = (float *)malloc(sizeof(float) * M * N); // CPU reference output
  if (!dst1) {
    printf("Error: cannot alloc memory\n");
    return 0;
  }

  // Transfer matrices A and B from system memory to URAM
  __tx_sync_uram_load(uram_src0, src0, M, K, 1, 1, K, 1, M * K, M * K);
  __tx_sync_uram_load(uram_src1, src1, K, N, 1, 1, N, 1, K * N, K * N);

  // Configure TCSR registers for tensor dimensions
  config_tcsr0(M, K); // Matrix A: M×K
  config_tcsr1(K, N); // Matrix B: K×N
  config_tcsr2(M, N); // Matrix C: M×N

  // Load matrix addresses into registers for tensor multiplication
  __asm__ volatile("mv x5, %0\n\t" : : "r"(uram_src0) : "x5"); // A address
  __asm__ volatile("mv x6, %0\n\t" : : "r"(uram_src1) : "x6"); // B address
  __asm__ volatile("mv x7, %0\n\t" : : "r"(uram_dst0) : "x7"); // C address

  // Execute tensor matrix multiplication instruction
  asm volatile(".byte 0x3f, 0x80, 0x62, 0x38, 0x88, 0x20, 0x08, 0x01");

  // Transfer result from URAM back to system memory
  __tx_sync_uram_store(dst0, uram_dst0, M, N, 1, 1, N, 1, M * N, M * N);

  // Compute reference result using CPU implementation
  matmul_cpu(src0, src1, dst1, M, K, N);

  // Compare accelerator result with CPU reference
  int passed = 1;
  for (i = 0; i < M * N; i++) {
    if (dst1[i] != dst0[i]) {
      printf("Test failed at element %d: %f != %f\n", i, dst1[i], dst0[i]);
      passed = 0;
      break;
    }
  }

  if (passed)
    printf("Passed\n");
  printf("----------Test end---------------\n\n");

  // Clean up allocated memory
  free(src0);
  free(src1);
  free(uram_src0);
  free(uram_src1);
  free(uram_dst0);
  free(dst0);
  free(dst1);

  return 0;
}

int main() {
  test_matmul();
  return 0;
}