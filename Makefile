ASM=nasm
QEMU=qemu-system-x86_64
SECTOR_SIZE=512
CONV=notrunc
BOOT_DIR = boot/
BOOT_BINS = $(BOOT_DIR)boot.bin $(BOOT_DIR)loader.bin $(BOOT_DIR)kernel.bin
# SRC_DIR=src
# BUILD_DIR=build

# bs=512 (sector size=512 bytes)
# count=1 (write 1 sector)
# conv=notrunc (not truncate the output file and size remains unchanged)
# seek=1 (skip the first sector which is boot.bin)
# $(word 2,$^) to get 2nd argument, loader.bin
# there're 1 sector for boot, 5 sectors for loader, and 100 sector for kernel
$(BOOT_DIR)boot.img: $(BOOT_BINS)
	dd if=$< of=$@ bs=$(SECTOR_SIZE) count=1 conv=$(CONV)
	dd if=$(word 2,$^) of=$@ bs=$(SECTOR_SIZE) count=5 seek=1 conv=$(CONV)
	dd if=$(word 3,$^) of=$@ bs=$(SECTOR_SIZE) count=100 seek=6 conv=$(CONV)

$(BOOT_DIR)boot.bin: $(BOOT_DIR)boot.asm
	$(ASM) -f bin $< -o $@

$(BOOT_DIR)loader.bin: $(BOOT_DIR)loader.asm
	$(ASM) -f bin $< -o $@

$(BOOT_DIR)kernel.bin: $(BOOT_DIR)kernel.asm
	$(ASM) -f bin $< -o $@

# pdpe1gb is for 1G huge page support
qemu: $(BOOT_DIR)boot.img
	$(QEMU) -cpu qemu64,pdpe1gb -drive format=raw,file=$<

.PHONY:	clean
clean: 
	rm -f $(BOOT_DIR)*.bin $(BOOT_DIR)*.img
