NAME = nanos-lite
SRCS = $(shell find -L ./src/ -name "*.c" -o -name "*.cpp" -o -name "*.S")
LIBS = klib
include $(AM_HOME)/Makefile.app

NANOS_MAX_SIZE = 0 #1048576
FSIMG_PATH = $(NAVY_HOME)/fsimg
FSIMG_FILES = $(shell find $(FSIMG_PATH) -type f)
RAMDISK_FILE = build/ramdisk.img

.PHONY: fsimg

fsimg:
	$(MAKE) -C $(NAVY_HOME) ISA=$(ISA)

$(FSIMG_FILES): fsimg

src/files.h: $(FSIMG_FILES)
	wc -c $(FSIMG_FILES) | grep -v 'total$$' | sed -e 's+ $(FSIMG_PATH)+ +' | awk -v sum=$(NANOS_MAX_SIZE) '{print "\x7b\x22" $$2 "\x22\x2c " $$1 "\x2c " sum "\x7d\x2c";sum += $$1}' > $@

src/initrd.S: $(RAMDISK_FILE)
	touch $@

$(RAMDISK_FILE): $(FSIMG_FILES)
	cat $^ > $@
