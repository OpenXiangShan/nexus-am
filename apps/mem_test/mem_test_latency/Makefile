NAME_1 = mem_test_latency
CFLAGS += -DINST=$(INST) -g
NAME = build-$(NAME_1)-$(INST)
$(NAME).c:mem_test_latency.c
	cp mem_test_latency.c $(NAME).c
SRCS = $(NAME).c
include $(AM_HOME)/Makefile.app
clean-build:
	rm -f build-*