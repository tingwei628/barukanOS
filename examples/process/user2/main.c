#include "../../../user/lib/lib.h"

int main(void)
{
    // buffer is to save commands
    char buffer[80] = { 0 };
    int32_t buffer_size = 0;
    int32_t cmd = 0;

    // press enter to reset buffer size to 0
    while (1)
    {
        printf("shell# ");

        // type command
        buffer_size = read_cmd(buffer);

        // there is no command
        if (buffer_size == 0)
        {
            continue;
        }
        
        cmd = parse_cmd(buffer, buffer_size);
        
        if (cmd < 0)
        {
            printf("Command Not Found!\n");
        }
        else
        {
            // press enter to command
            execute_cmd(cmd);             
        }            
    }

    return 0;
}