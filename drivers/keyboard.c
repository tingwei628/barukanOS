#include "keyboard.h"
#include "port.h"
#include "../kernel/process.h"

// extern uint8_t in_byte(uint16_t port);

static uint8_t shift_code[256] =
{
    [0x2A] = SHIFT, // Shift key (left) down
    [0x36] = SHIFT, // Shift key (right) down
    [0xAA] = SHIFT, // Shift key (left) up
    [0xB6] = SHIFT  // Shift key (right) up
};

static uint8_t lock_code[256] =
{
    [0x3A] = CAPS_LOCK
};

// if not press shift key
// scan codes to convert to ascii codes
static char key_map[256] =
{
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '-', '=', '\b', 0, 'q', 'w', 'e', 'r', 't', 'y', 'u',
    'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's', 'd', 'f',
    'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z',
    'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

// press shift key and press another key
static char shift_key_map[256] =
{
    0, 1, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
    '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U',
    'I', 'O', 'P', '{', '}', '\n', 0, 'A', 'S', 'D', 'F', 'G',
    'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C',
    'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
};

// key_buffer is a circular queue
static KeyboardBuffer key_buffer = 
{ 
    .buffer = {0},
    .front = 0,
    .end = 0,
    .size = 500
};

static uint32_t flag;

// write key into key_buffer
static void write_key_buffer(char ch)
{
    // key_buffer is a circular queue
    int32_t front = key_buffer.front;
    int32_t end = key_buffer.end;
    int32_t size = key_buffer.size;

    // if key_buffer is full
    if ((end + 1) % size == front)
    {
        return;
    }

    key_buffer.buffer[end++] = ch;
    key_buffer.end = end % size;
}

static char keyboard_read(void)
{
    uint8_t scan_code;
    char ch;

    // 0x60 = data port (read/write)
    scan_code = in_byte(0x60);
    
    // E0_SIGN = Output buffer status (0 = empty, 1 = full)
    // must be set before attempting to read data from IO port 0x60

    // it is just a part of scan codes like function keys
    if (scan_code == 0xE0)
    {
        flag |= E0_SIGN;   
        return 0;
    }

    // last part of scan codes is e0
    // it means it's a function key not used in system
    if (flag & E0_SIGN)
    {
        // clear up e0 flag
        flag &= ~E0_SIGN;
        return 0;
    }

    // not handling pause key here

    // if key up (0x80 = bit-7 is 1)
    if (scan_code & 0x80)
    {
        // shift key up and clear shift key flag
        flag &= ~(shift_code[scan_code]);
        return 0;
    }

    // set if shift key is pressed
    flag |= shift_code[scan_code];
    // switch current lock bit
    flag ^= lock_code[scan_code];

    // if shift key is pressed
    if (flag & SHIFT)
    {
        ch = shift_key_map[scan_code];
    }
    else
    {
        ch = key_map[scan_code];
    }

    // if caps key is locked
    if (flag & CAPS_LOCK)
    {      
        // lower case to upper case
        if('a' <= ch && ch <= 'z')
            ch -= 32;

        // upper case to lower case
        else if('A' <= ch && ch <= 'Z')
            ch += 32;
    }

    return ch;
}

// read key from key_buffer
char read_key_buffer(void)
{
    int8_t front = key_buffer.front;

    // there is no key to key in key_buffer
    if (front == key_buffer.end)
    {
        //put process into sleep
        // -2 = process waiting for keyboard IO
        sleep(-2);       
    }
    // update next to front
    key_buffer.front = (key_buffer.front + 1) % key_buffer.size;
    return key_buffer.buffer[front];
}

void keyboard_handler(void)
{
    // read from keyboard io
    char ch = keyboard_read();

    // if ch is a valid char
    if (ch > 0)
    {
        // write into key buffer
        write_key_buffer(ch);
        // if there is no key to read in  key buffer, and put process into sleep
        // otherwise process wakes up to read
        // -2 = process waiting for keyboard IO
        wake_up(-2);
    }
}


