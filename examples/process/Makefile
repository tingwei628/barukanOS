CC = gcc
CFLAGS = -std=c11 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -g
USER_DIR = user/
USER_LIB_DIR = $(USER_DIR)lib/
EXAMPLES_DIR = examples/process/

.PHONY:	clean examples

examples: $(EXAMPLES_DIR)user1.bin $(EXAMPLES_DIR)user2.bin $(EXAMPLES_DIR)user3.bin
	@echo "end of building examples..."

$(EXAMPLES_DIR)user3.bin: $(EXAMPLES_DIR)user3/user3
	objcopy -O binary $< $@

$(EXAMPLES_DIR)user2.bin: $(EXAMPLES_DIR)user2/user2
	objcopy -O binary $< $@

$(EXAMPLES_DIR)user1.bin: $(EXAMPLES_DIR)user1/user1
	objcopy -O binary $< $@

$(EXAMPLES_DIR)user3/user3: $(EXAMPLES_DIR)user3/start.o $(EXAMPLES_DIR)user3/main.o $(USER_LIB_DIR)lib.a
	ld -nostdlib -T $(EXAMPLES_DIR)linker.ld -o $@ $^

$(EXAMPLES_DIR)user2/user2: $(EXAMPLES_DIR)user2/start.o $(EXAMPLES_DIR)user2/main.o $(USER_LIB_DIR)lib.a
	ld -nostdlib -T $(EXAMPLES_DIR)linker.ld -o $@ $^

$(EXAMPLES_DIR)user1/user1: $(EXAMPLES_DIR)user1/start.o $(EXAMPLES_DIR)user1/main.o $(USER_LIB_DIR)lib.a
	ld -nostdlib -T $(EXAMPLES_DIR)linker.ld -o $@ $^

$(EXAMPLES_DIR)%/start.o: $(EXAMPLES_DIR)%/start.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(EXAMPLES_DIR)%/main.o: $(EXAMPLES_DIR)%/main.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(EXAMPLES_DIR)*/*.o $(EXAMPLES_DIR)*/*.bin $(EXAMPLES_DIR)user1/user1 \
	$(EXAMPLES_DIR)user2/user2 $(EXAMPLES_DIR)user3/user3
	