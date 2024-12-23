ASM = nasm
QEMU = qemu-system-x86_64
CC = gcc
CFLAGS = -std=c11 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -mgeneral-regs-only -g
ASMFLAGS = -f elf64 -g -F dwarf
SECTOR_SIZE = 512
CONV = notrunc
BOOT_DIR = boot/
USER_DIR = user/
USER_LIB_DIR = $(USER_DIR)lib/
KERNEL_DIR = kernel/
KERNEL_OBJS = $(KERNEL_DIR)kernel.o $(KERNEL_DIR)main.o $(KERNEL_DIR)idt.o \
	$(KERNEL_DIR)string.o $(KERNEL_DIR)memory.o $(KERNEL_DIR)mem.o $(KERNEL_DIR)print.o $(KERNEL_DIR)debug.o \
	$(KERNEL_DIR)vm.o $(KERNEL_DIR)process.o $(KERNEL_DIR)syscall.o $(KERNEL_DIR)list.o \
	$(KERNEL_DIR)trap.o

DRIVERS_DIR = drivers/
DRIVERS_OBJS = $(DRIVERS_DIR)keyboard.o $(DRIVERS_DIR)port.o

USER_DIR = user/
EXAMPLES_DIR = examples/
EXAMPLES_PROCESS_DIR = $(EXAMPLES_DIR)process/
EXAMPLES_FS_DIR = $(EXAMPLES_DIR)fs/

FS_DIR = fs/
FS_FAT16_DIR = $(FS_DIR)fat16/
FS_OBJS = $(FS_FAT16_DIR)impl.o $(FS_FAT16_DIR)init.o

# bs=512 (sector size=512 bytes)
# count=1 (write 1 sector)
# conv=notrunc (not truncate the output file and size remains unchanged)
# seek=1 (skip the first sector which is boot.bin)
# $(word 2,$^) to get 2nd argument, loader.bin
# there're 1 sector for boot, 5 sectors for loader, and 100 sector for kernel


.PHONY:	clean lib lib-clean os-boot os-loader build-kernel

build-kernel: $(BOOT_DIR)kernel.bin
	@echo "end of building a kernel..."

$(BOOT_DIR)kernel.bin: $(KERNEL_DIR)kernel
	objcopy -O binary $^ $@

$(KERNEL_DIR)kernel: $(KERNEL_OBJS) $(DRIVERS_OBJS) $(FS_OBJS)
	ld -nostdlib -T $(KERNEL_DIR)linker.ld -o $@ $^

$(KERNEL_DIR)kernel.o: $(KERNEL_DIR)kernel.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(KERNEL_DIR)trap.o: $(KERNEL_DIR)trap.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(DRIVERS_DIR)%.o: $(DRIVERS_DIR)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(FS_FAT16_DIR)%.o: $(FS_FAT16_DIR)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(KERNEL_DIR)%.o: $(KERNEL_DIR)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

os-boot: 
	@echo "write boot into os..."
	$(MAKE) -f $(BOOT_DIR)Makefile os-boot

os-loader:
	@echo "write loader into os..."
	$(MAKE) -f $(BOOT_DIR)Makefile os-loader

boot-clean:
	@echo "clean boot..."
	$(MAKE) -f $(BOOT_DIR)Makefile clean

lib: 
	@echo "build user lib..."
	$(MAKE) -f $(USER_DIR)Makefile lib

lib-clean:
	@echo "clean user lib..."
	$(MAKE) -f $(USER_DIR)Makefile clean

process-examples:
	@echo "build process examples..."
	$(MAKE) -f $(EXAMPLES_PROCESS_DIR)Makefile examples

process-examples-clean:
	@echo "clean process examples..."
	$(MAKE) -f $(EXAMPLES_PROCESS_DIR)Makefile clean

fat16-examples:
	@echo "build fat16 examples..."
	$(MAKE) -f $(EXAMPLES_FS_DIR)Makefile examples

fat16-examples-clean:
	@echo "clean fat16 examples..."
	$(MAKE) -f $(EXAMPLES_FS_DIR)Makefile clean

qemu-os: $(BOOT_DIR)os.img
	@echo "qemu + os..."
	$(QEMU) -drive format=raw,file=$< -m 1024M

clean: 
	@echo "clean..."
	rm -f **/*.bin boot/user.img boot/boot.img **/*.o $(KERNEL_DIR)kernel
