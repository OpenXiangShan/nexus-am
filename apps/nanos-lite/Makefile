NAME = nanos-lite
SRCS = $(shell find -L ./src/ -name "*.c" -o -name "*.cpp" -o -name "*.S")
LIBS = klib
include $(AM_HOME)/Makefile.app

FSIMG_PATH = $(NAVY_HOME)/fsimg
FSIMG_FILES = $(shell find $(FSIMG_PATH) -type f)
RAMDISK_FILE = build/ramdisk.img

.PHONY: update

update-fsimg:
	$(MAKE) -C $(NAVY_HOME) ISA=$(ISA)
	for f in $(FSIMG_FILES); do \
		if $(READELF) -h $$f 2> /dev/null > /dev/null; then \
			$(OBJCOPY) --set-section-flags .bss=alloc,contents -O binary $$f; \
		fi \
	done

src/files.h: update-fsimg
	@wc -c $(FSIMG_FILES) | grep -v 'total$$' | sed -e 's+ $(FSIMG_PATH)+ +' | awk -v sum=0 '{print "\x7b\x22" $$2 "\x22\x2c " $$1 "\x2c " sum "\x7d\x2c";sum += $$1}' > $@

src/initrd.S: $(RAMDISK_FILE)
	touch $@

src/syscall.h: $(NAVY_HOME)/libs/libos/src/syscall.h
	ln -sf $^ $@

$(RAMDISK_FILE): update-fsimg
	@cat $(FSIMG_FILES) > $@

#$(RAMDISK_FILE): $(NAVY_HOME)/tests/hello/build/hello-x86
#	$(OBJCOPY) --set-section-flags .bss=alloc,contents -O binary $^ $@

update: src/initrd.S src/syscall.h src/files.h
