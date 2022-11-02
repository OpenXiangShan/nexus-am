NAME_1 = fcomtest
#CFLAGS+=-DINST=$(INST) -DCHOOSEO=$(CHOOSEO) -DCHOOSET=$(CHOOSET)
#CHOOSEO ?= 1
#CHOOSET ?= 3
CFLAGS +=-DCHOOSEO=$(CHOOSEO) -g 
CFLAGS +=-DCHOOSET=$(CHOOSET) -g
CFLAGS +=-DCHOOSER=$(CHOOSET) -g
NAME = build-$(NAME_1)-$(CHOOSEO)-$(CHOOSET)-$(CHOOSER)
$(NAME).c:fcomtest.c
	cp fcomtest.c $(NAME).c
SRCS = $(NAME).c
include $(AM_HOME)/Makefile.app
clean-build:
	rm -f build-*