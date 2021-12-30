ASM=nasm
QEMU=qemu-system-x86_64
# SRC_DIR=src
# BUILD_DIR=build

boot.img: boot.bin
	dd if=$< of=$@ bs=512 count=1 conv=notrunc

boot.bin: boot.asm
	$(ASM) -f bin $< -o $@

qemu: boot.img
	# $(QEMU) -fda $<
	$(QEMU) -drive format=raw,file=$< 

.PHONY:	clean
clean: 
	rm -f *.bin *.img
