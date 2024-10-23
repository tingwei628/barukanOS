CC = gcc
CFLAGS = -std=c11 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -g
AR = ar
ASM = nasm
ASMFLAGS = -f elf64 -g -F dwarf
USER_DIR = user/
USER_LIB_DIR = $(USER_DIR)lib/
USER_OBJS = $(USER_LIB_DIR)lib.o $(USER_LIB_DIR)print.o $(USER_LIB_DIR)console.o $(USER_LIB_DIR)memory.o

lib: $(USER_LIB_DIR)lib.a
	@echo "end of build user lib..."

$(USER_LIB_DIR)lib.a: $(USER_OBJS)
	$(AR) -rcs $@ $<

$(USER_LIB_DIR)%.o: $(USER_LIB_DIR)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(USER_LIB_DIR)lib.o: $(USER_LIB_DIR)lib.asm
	$(ASM) $(ASMFLAGS) $< -o $@


.PHONY:	clean
clean: 
	rm -f $(USER_DIR)*.o $(USER_LIB_DIR)*.o $(USER_LIB_DIR)*.a