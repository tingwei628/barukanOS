# loader
ASM = nasm
QEMU = qemu-system-x86_64
CC = gcc
CFLAGS = -std=c11 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -mgeneral-regs-only -g
ASMFLAGS = -f elf64 -g -F dwarf
SECTOR_SIZE = 512
CONV = notrunc

BOOT_DIR = boot/
KERNEL_DIR = kernel/
FS_DIR = fs/
FS_FAT16_DIR = $(FS_DIR)fat16/

LOADER_OBJS = $(BOOT_DIR)loader_entry.o $(BOOT_DIR)loader_main.o $(BOOT_DIR)loader_fs.o $(KERNEL_DIR)print.o \
	$(KERNEL_DIR)debug.o $(FS_FAT16_DIR)impl.o $(KERNEL_DIR)string.o $(KERNEL_DIR)mem.o

.PHONY:	clean os-boot os-loader

os-boot: $(BOOT_DIR)boot.bin
	dd if=$< of=$(BOOT_DIR)os.img bs=$(SECTOR_SIZE) count=1 conv=$(CONV)

os-loader: $(BOOT_DIR)loader.bin
	dd if=$< of=$(BOOT_DIR)os.img bs=$(SECTOR_SIZE) count=30 seek=1 conv=$(CONV)

$(BOOT_DIR)boot.bin: $(BOOT_DIR)boot.asm
	$(ASM) -f bin $< -o $@

$(BOOT_DIR)loader_main.o: $(BOOT_DIR)loader_main.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(BOOT_DIR)loader_fs.o: $(BOOT_DIR)loader_fs.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(BOOT_DIR)loader.bin: $(BOOT_DIR)loader.asm $(BOOT_DIR)loader_entry.bin
	$(ASM) -f bin $< -o $@
	dd if=$(word 2,$^) >> $@
	
$(BOOT_DIR)loader_entry.bin: $(BOOT_DIR)loader_entry
	objcopy -O binary $^ $@

$(BOOT_DIR)loader_entry: $(LOADER_OBJS)
	ld -nostdlib -T loader.ld -o $@ $^

$(BOOT_DIR)loader_entry.o: $(BOOT_DIR)loader_entry.asm
	$(ASM) $(ASMFLAGS) $< -o $@

clean: 
	@echo "boot clean..."
	rm -f $(BOOT_DIR)*.bin $(BOOT_DIR)user.img $(BOOT_DIR)boot.img $(BOOT_DIR)*.o $(BOOT_DIR)loader_entry