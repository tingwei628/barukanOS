#include "../../../../user/lib/lib.h"

int main(void)
{
    int32_t fd;
    int32_t size;
    char buffer[100] = { 0 };

    // since name and ext of DirEntry are uppercases
    fd = open_file("DATA.BIN");

    if (fd == -1)
    {
        printf("Open file failed in new process of user2\n");
    }
    else 
    {
        size = get_file_size(fd);
        size = read_file(fd, buffer, size);

        if (size != -1)
        {
            printf("%s\n", buffer);
            printf("read %db in total\n", size);
        }
        close_file(fd);
    }

    return 0;
}