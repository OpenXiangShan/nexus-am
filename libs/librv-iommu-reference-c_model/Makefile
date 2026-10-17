NAME = librv-iommu-reference-c_model

SRCS = wrapper.c utils.c queue.c \
       $(shell find libiommu/src libtables/src -name "*.c")

INC_DIR += $(LIB_DIR)
CFLAGS += -Wno-unused-but-set-variable -Wno-unused-variable \
          -Wno-maybe-uninitialized

include $(AM_HOME)/Makefile.lib
