#include "stdint.h"
#include "stddef.h"

typedef void (*CmdFunc)(void);
extern uint8_t keyboard_readu(void);
extern int32_t get_total_memoryu(void);
extern uint8_t memcmp_u(void *src1, void *src2, size_t size);
extern int32_t printf(const uint8_t *format, ...);


static void cmd_get_total_memory(void)
{
    uint64_t total;
    
    total = get_total_memoryu();
    printf("Total Memory is %dMB\n", total);
}

int32_t read_cmd(char *buffer)
{
    char ch[2] = { 0 };
    int32_t buffer_size = 0;

    while (1)
    {
        // read a char from keyboard
        ch[0] = keyboard_readu();
        
        // press enter key
        if (ch[0] == '\n' || buffer_size >= 80)
        {
            printf("%s", ch);
            break;
        }
        // press backspace key (delete char)
        else if (ch[0] == '\b')
        {    
            // buffer size is 0 means beginning of line
            if (buffer_size > 0)
            {
                buffer_size--;
                // print backspace
                printf("%s", ch);    
            }           
        }          
        else
        {     
            buffer[buffer_size++] = ch[0]; 
            printf("%s", ch);        
        }
    }

    return buffer_size;
}

int32_t parse_cmd(char *buffer, int32_t buffer_size)
{
    int32_t cmd = -1;

    // command is "totalmem"
    if (buffer_size == 8 && (!memcmp_u("totalmem", buffer, 8)))
    {
        cmd = 0;
    }

    return cmd;
}

void execute_cmd(int32_t cmd)
{
    CmdFunc cmd_list[1] = { cmd_get_total_memory };

    if (cmd == 0)
    {       
        cmd_list[0]();
    }
}


