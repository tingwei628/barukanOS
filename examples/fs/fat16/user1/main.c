#include "../../../../user/lib/lib.h"

int main(void)
{
    int32_t fd;
    int32_t size;
    char buffer[100] = {0};

    while (1)
    {
        printf("shell# ");
        memset_u(buffer, 0, 100);
        size = read_cmd(buffer);
        
        if (size == 0)
        {
            continue;
        }

        //USER2.BIN
        fd = open_file(buffer);

        if (fd == -1)
        {
            printf("Command Not Found\n");
        }
        else
        {
            close_file(fd);

            int32_t pid = fork();
                        
            if (pid == 0)
            {
                // in child process
                // printf("a new process\n");
                exec(buffer);
            }
            else if(pid > 0)
            {
                // in parent process
                // printf("a current process\n");
                waitu(pid);
            }
            else 
            {
                printf("fork failed\n");
            }
        }
    }

    return 0;
}