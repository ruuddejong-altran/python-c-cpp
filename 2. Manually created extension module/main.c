#include <stdio.h>
#include "spamlib.h"

int subtract(int x, int y)
{
    printf("C subtract is called with (%d, %d)\n", x, y);
    return x - y;
}

int main()
{
    int x = 3;
    int y = 5;
    printf("x = %d, y = %d\n", x, y);

    int result = add(x, y);
    printf("spamlib.add(%d, %d) gives %d\n", x, y, result);

    swap(&x, &y);
    printf("After spamlib.swap(), x = %d, y = %d\n", x, y);

    result = do_operation(x, y, &subtract);
    printf("do_operation(%d, %d, &subtract) gives %d\n", x, y, result);

    return 0;
}