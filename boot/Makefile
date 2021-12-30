ASM=nasm
QEMU=qemu-system-x86_64
SECTOR_SIZE=512
CONV=notrunc
# SRC_DIR=src
# BUILD_DIR=build

# bs=512 (sector size=512 bytes)
# count=1 (write 1 sector)
# conv=notrunc (not truncate the output file and size remains unchanged)
# seek=1 (skip the first sector which is boot.bin)
# $(word 2,$^) to get 2nd argument, loader.bin
boot.img: boot.bin loader.bin
	dd if=$< of=$@ bs=$(SECTOR_SIZE) count=1 conv=$(CONV)
	dd if=$(word 2,$^) of=$@ bs=$(SECTOR_SIZE) count=1 seek=1 conv=$(CONV)

boot.bin: boot.asm
	$(ASM) -f bin $< -o $@

loader.bin: loader.asm
	$(ASM) -f bin $< -o $@

# pdpe1gb is for 1G huge page support
qemu: boot.img
	$(QEMU) -cpu qemu64,pdpe1gb -drive format=raw,file=$<

.PHONY:	clean
clean: 
	rm -f *.bin *.img
