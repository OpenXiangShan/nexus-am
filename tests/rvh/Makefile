CROSS_COMPILE ?= riscv64-linux-gnu-
BUILD_DIR := build
TESTS := two-stage-mbmc
ELFS := $(addprefix $(BUILD_DIR)/,$(addsuffix .elf,$(TESTS)))
BINARIES := $(ELFS:.elf=.bin)

.PHONY: all clean

all: $(ELFS) $(BINARIES)

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.elf: %.S link.ld | $(BUILD_DIR)
	$(CROSS_COMPILE)gcc -march=rv64im_zicsr -mabi=lp64 -fno-pie -no-pie \
		-nostdlib -nostartfiles \
		-static -Wl,--build-id=none -Wl,-T,link.ld -o $@ $<

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	$(CROSS_COMPILE)objcopy -O binary $< $@

clean:
	rm -rf $(BUILD_DIR)
