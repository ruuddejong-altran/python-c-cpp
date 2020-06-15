#include "spamlib.h"

int add(int a, int b)
{
    return a + b;
}

void swap(int& a, int& b)
{
    int tmp = a;
    a = b;
    b = tmp;
}

int do_operation(int a, int b, std::function<int(int, int)> operator_func)
{
    return operator_func(a, b);
}
