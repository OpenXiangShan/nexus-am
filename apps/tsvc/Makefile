NAME = tsvc
SRCS = $(shell find -L ./src/ -name "*.c")
CFLAGS := -O3 -fstrict-aliasing -fivopts -ftree-vectorize
MARCH ?= rv64gcv_zba_zbb_zbc_zbs_zbkb_zbkc_zbkx_zknd_zkne_zknh_zkr_zksed_zksh_zkt
include $(AM_HOME)/Makefile.app
