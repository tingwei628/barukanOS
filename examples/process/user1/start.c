#include "../../../user/lib/lib.h"

extern int main(void);
extern void exitu(void);

void start(void)
{
    main();
    exitu();
    while (1) {}
}