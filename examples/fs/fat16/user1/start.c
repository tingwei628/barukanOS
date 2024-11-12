#include "../../../../user/lib/lib.h"

extern int main(void);

void start(void)
{
    main();
    exitu();
    while (1) {}
}