#include "spamlib.h"

int add(int a, int b)
{
    return a + b;
}

void swap(int* a, int* b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

int do_operation(int a, int b, int (*operation)(int a, int b))
{
    return operation(a, b);
}
