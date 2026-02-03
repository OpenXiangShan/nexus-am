CASE = vsha256
SRCS = \
	main.c \
	vsha256_single.S \
	vsha256_multi.S \

N ?= 56
CFLAGS += -DN=$(N)

ITERATIONS ?= 1
CFLAGS += -DTIMES=$(ITERATIONS)

NAME = $(CASE)-$(N)B-$(ITERATIONS)ITERATIONS

MARCH := rv64gcv_zba_zbb_zbc_zbs_zbkb_zbkc_zbkx_zknd_zkne_zknh_zkr_zksed_zksh_zkt_zvknha_zvkb

$(info ${CFLAGS})

include $(AM_HOME)/Makefile.app
