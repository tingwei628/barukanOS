#include "../../../../user/lib/lib.h"

DirEntry buffer[1024];

int main(void)
{
    int32_t count;
    char name[10] = {0};

    count = read_root_directory(buffer);
    
    if (count != 0)
    {
        printf("\nName  Dir     File size \n");
        printf("-------------------------\n");
        for (int32_t i = 0; i < count; i++)
        {
            if (buffer[i].name[0] == EMPTY ||
                buffer[i].name[0] == DELETED ||
                buffer[i].attributes == LONG_FILE_NAME ||
                buffer[i].attributes == VOLUME_LABEL)
            {
                continue;
            }

            memcpy_u(name, buffer[i].name, 8);
            if ((buffer[i].attributes & DIRECTORY) == DIRECTORY) 
            {
                printf("%s  Yes     %ukb\n", name, (uint64_t)buffer[i].file_size/1024);
            }
            else
            {
                printf("%s  No     %ukb\n", name, (uint64_t)buffer[i].file_size/1024);
            }
        }
    }
    return 0;
}