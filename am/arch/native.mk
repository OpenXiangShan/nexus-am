AM_SRCS := native/trm.c \
           native/ioe.c \
           native/cte.c \
           native/trap.S \
           native/vme.c \
           native/platform.cpp \
           native/devices/input.c \
           native/devices/timer.c \
           native/devices/video.c \

image:
	@echo + LD "->" $(BINARY)
	@g++ -pie -o $(BINARY) -Wl,--start-group $(LINK_FILES) -Wl,--end-group -lSDL2 -lGL -lrt

run:
	$(BINARY)