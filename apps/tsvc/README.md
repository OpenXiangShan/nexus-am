# TSVC 2

Updated version of TSVC to capture the same information but hopefully in a better format to make it easier to add things. We've fixed a few bugs along the way too.

The original paper which laid out the details of the suite and provided results (D. Callahan, J. Dongarra, and D. Levine. Vectorizing compilers: a test suite and results. Proceedings. Supercomputing ’88) originally had the loops in Fortran.

The C version of the benchmark used as a base for TSVC-2 was found [here](http://polaris.cs.uiuc.edu/~maleki1/TSVC.tar.gz). The old C version had some problems to do with compilers being able to inline all of the initialisation and checksum loops, which wildly skews the results and the timing. There are also some bugs with writing off the end of arrays and some of the initialisation routines seem to not initialise the right arrays. 

This version should (hopefully) be free(er) of bugs and should stop the compiler from being able to completely eliminate code.

## Original Repository

https://github.com/UoB-HPC/TSVC_2

## Changes

To perform the test, we made the following modifications to the original program:

1. We modified the makefile to enable compilation in the AM environment.

2. We reduced the value of the iterations variable in common.h (from 100,000 to 100) to shorten the runtime.

3. We added the library files required for the test (undefined_lib.h, undefined_lib.c), specifically the sinf and cosf functions needed for test s451. This may lead to performance and result differences compared to running on a Linux system.

The tests show that the benchmark can run successfully on the riscv64-xs architecture in NEMU.