#ifndef PYTHON_C_CPP
#define PYTHON_C_CPP

#include <functional>

int add(int a, int b);
void swap(int& a, int& b);
int do_operation(int a, int b, std::function<int(int, int)> operator_func);

#endif //PYTHON_C_CPP
