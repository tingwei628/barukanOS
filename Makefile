ASM = nasm
QEMU = qemu-system-x86_64
CC = gcc
CFLAGS = -std=c11 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -mgeneral-regs-only -g
ASMFLAGS = -f elf64 -g -F dwarf
SECTOR_SIZE = 512
CONV = notrunc
BOOT_DIR = boot/
BOOT_BINS = $(BOOT_DIR)boot.bin $(BOOT_DIR)loader.bin $(BOOT_DIR)kernel.bin
KERNEL_DIR = kernel/
KERNEL_OBJS = $(BOOT_DIR)kernel.o $(KERNEL_DIR)main.o $(KERNEL_DIR)idt.o \
	$(KERNEL_DIR)string.o $(KERNEL_DIR)memory.o $(KERNEL_DIR)print.o $(KERNEL_DIR)debug.o \
	$(KERNEL_DIR)vm.o $(KERNEL_DIR)process.o $(KERNEL_DIR)syscall.o $(KERNEL_DIR)list.o \
	$(KERNEL_DIR)trap.o

USER_DIR = user/
EXAMPLES_DIR = examples/process/
EXAMPLES_BINS = $(EXAMPLES_DIR)/user1.bin $(EXAMPLES_DIR)/user2.bin $(EXAMPLES_DIR)/user3.bin
USER_BINS = $(BOOT_BINS) $(EXAMPLES_BINS)
# bs=512 (sector size=512 bytes)
# count=1 (write 1 sector)
# conv=notrunc (not truncate the output file and size remains unchanged)
# seek=1 (skip the first sector which is boot.bin)
# $(word 2,$^) to get 2nd argument, loader.bin
# there're 1 sector for boot, 5 sectors for loader, and 100 sector for kernel

2
define write_boot_bin
	dd if=$< of=$@ bs=$(SECTOR_SIZE) count=1 conv=$(CONV)
	dd if=$(word 2,$^) of=$@ bs=$(SECTOR_SIZE) count=5 seek=1 conv=$(CONV)
	dd if=$(word 3,$^) of=$@ bs=$(SECTOR_SIZE) count=100 seek=6 conv=$(CONV)
endef

.PHONY:	clean lib lib-clean examples examples-clean

# 10321920 = 20 * 16 * 63 * 512
$(BOOT_DIR)user.img: $(USER_BINS)
	$(call write_boot_bin)
	dd if=$(word 4,$^) of=$@ bs=$(SECTOR_SIZE) count=10 seek=106 conv=$(CONV)
	dd if=$(word 5,$^) of=$@ bs=$(SECTOR_SIZE) count=10 seek=116 conv=$(CONV)
	dd if=$(word 6,$^) of=$@ bs=$(SECTOR_SIZE) count=10 seek=126 conv=$(CONV)
	truncate -s 10321920 $@

$(BOOT_DIR)boot.img: $(BOOT_BINS)
	$(call write_boot_bin)

$(BOOT_DIR)boot.bin: $(BOOT_DIR)boot.asm
	$(ASM) -f bin $< -o $@

$(BOOT_DIR)loader.bin: $(BOOT_DIR)loader.asm
	$(ASM) -f bin $< -o $@

$(BOOT_DIR)kernel.bin: $(KERNEL_DIR)kernel
	objcopy -O binary $^ $@

$(KERNEL_DIR)kernel: $(KERNEL_OBJS)
	ld -nostdlib -T $(KERNEL_DIR)linker.ld -o $@ $^

$(BOOT_DIR)kernel.o: $(BOOT_DIR)kernel.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(KERNEL_DIR)trap.o: $(KERNEL_DIR)trap.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(KERNEL_DIR)%.o: $(KERNEL_DIR)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

lib: 
	@echo "build user lib..."
	$(MAKE) -f $(USER_DIR)Makefile lib

lib-clean:
	@echo "clean user lib..."
	$(MAKE) -f $(USER_DIR)Makefile clean

examples:
	@echo "build examples..."
	$(MAKE) -f $(EXAMPLES_DIR)Makefile examples

examples-clean:
	@echo "clean examples..."
	$(MAKE) -f $(EXAMPLES_DIR)Makefile clean

# pdpe1gb is for 1G huge page support
qemu: $(BOOT_DIR)boot.img
	@echo "qemu..."
	$(QEMU) -cpu qemu64,pdpe1gb -drive format=raw,file=$<

# add user space program
qemu-user: $(BOOT_DIR)user.img
	@echo "qemu-user..."
	$(QEMU) -cpu qemu64,pdpe1gb -drive format=raw,file=$<

clean: 
	@echo "clean..."
	rm -f **/*.bin **/*.img **/*.o $(KERNEL_DIR)kernel
