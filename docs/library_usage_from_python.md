---
layout: default
title: Basic C library usage
---

# Basic C library usage

When you work in Python,
you sometimes have a need for functionality
that you know is available in a C or C++ library.
This is such a common scenario that most popular
libraries nowadays already have
Python wrappers that make them directly usable in Python.
Examples are `pyqt` and `pyzmq`, to name just two at opposite
sides of the application spectrum (GUI development versus low-level
network connectivity).

But sometimes all you have is the library
and no ready-made Python wrapper.
Even in such a case you can use the library in Python - it just takes
a little more effort.

For the following examples I created a C library `spamlib` with three publicly
available functions:

```c
int add(int a, int b);
void swap(int* a, int* b);
int do_operation(int a, int b, int (*operator)(int a, int b));
```
* Function `add`, as the name suggests, adds the two integer arguments and returns the result.
  This illustrates basic argument passing.
* Function `swap` takes two pointers to integers, and swaps their values.
  This illustrates that simple argument passing is not enough in this case,
  because Python does not have the concept of pointer.
  The only way to make a Python variable refer to another object
  is to re-assign the original variable.
* Function `do_operation` illustrates the use of a (Python) function as a callback functions
  that is called from within a C function.

The `spamlib` library sourcecode is small enough that it can be reproduced here:

```c
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

int do_operation(int a, int b, int (*operator)(int a, int b))
{
    return operator(a, b);
}
```

And to illustrate that this works in C, file `main.c` exercises the functions
in this library, and reports the results:

```c
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
```

The following examples show a few different ways in which this library can be
used from within Python.
Each example also contains a Python file `demo.py` that illustrates the use
of the library, similar the the C `main.c` file shown above.
To execute the `demo.py` file, `cd` to the proper build directory
(the one that has `spamlib.dll` or `libspamlib.so`),
and execute `python3 demo.py`.

 * [Example 1 - Using ctypes](./example_1.md). This shows a pure Python solution
   for accessing a C library.
 
 * [Example 2 - Manually created extension module](./example_2.md). This shows
   how to code an extension module in C, so that the result can be used directly in Python
  
 * [Example 3-1 - SWIG generated extension module](example_3-1.md). This shows how
   SWIG can be used to generate an interface wrapper.
  
 * [Example 3-2 - PyBind11 generated extension module](example_3-2.md). This shows how
   `pybind11`  can be used to generate an interface wrapper.