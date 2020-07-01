---
layout: default
title: Type Conversions
---

# Type Conversions

One of the difficulties in bridging C++ and Python
is that the object layouts differ.
In order to use native Python types
from C++, or native C++ types from Python,
the binding code that provides the bridge
between the two languages
must have provisions to make this possible.

There are three fundamentally different ways to do this:

* Use a native C++ type everywhere, and provide wrapping code
  so that Python can use it;
* Use a native Python type everywhere, and provide wrapping code
  so that C++ can use it;
* Use a native C++ type on the C++ side, and a native Python type on
  the Python side, and convert values on-the-fly.
  
Each of these methods has its pros and cons.
Which method is most appropriate depends very much
on the actual situation you're dealing with.

The third option, type conversion, often feels the most natural,
because you are dealing with native types on both the Python and C++ side.
But since the objects are converted, you actually have two independent, decoupled
representations of the same value.
Any change you make on one side of the bridge will not
propagate to the other side.

As an example, consider the following C++ function
that takes a reference to a `std::vector`
and adds an element to it.
It also prints the contents of the vector before
and after the modification,
to show what is going on.

```c
template <typename T>
void add_to_sequence(std::vector<T>& seq, T value)
{
    std::cout << "Before: ";
    print_sequence(seq);
    seq.push_back(value);
    std::cout << "After: ";
    print_sequence(seq);
}
```

For completeness sake, the function `print_sequence` is defined as follows:

```c
template<typename T>
void print_sequence(std::vector<T> seq)
{
    std::cout << "[";
    if (!seq.empty())
    {
        std::cout << seq[0];
        std::for_each(++seq.begin(), seq.end(),
                      [](const auto& value)
                      {
                          std::cout << ", " << value;
                      });
    }
    std::cout << "]" << std::endl;
}
```

We can use this in a toy C++ example:

```c
int main()
{
    /* In-place extension of sequence */
    std::vector<int> int_seq {0, 1, 2};
    add_to_sequence(int_seq, 4);
    std::cout << "In main: ";
    print_sequence(int_seq);
    return 0;
}
```

When we use this function from C++,
everything works as expected.
The `std::vector` instance in `main` does indeed
have the extra element at the end:

```text
Before: [0, 1, 2]
After: [0, 1, 2, 4]
In main: [0, 1, 2, 4]

Process finished with exit code 0
```

We can make this function available in Python as follows:

```c
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/iostream.h"

#include "playgroundlib.h"

namespace py = pybind11;

PYBIND11_MODULE(conversion, m)
{
    ...
    m.def("add_to_sequence", &add_to_sequence<int>,
          py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    ...
}
```

Note that we have to specify the actual template instantiation(s) we want
to have available.
The third argument in the `m.def` call serves to temporarily
redirect the C++ standard output and standard error streams
to their Python counterparts.
If we don't do this, the output from the `print_sequence` calls
in `add_to_sequence` are not shown in Python.

When we build this and run the following Python code,
we see that the C++ code works fine,
but the result is not propagated back to Python:

```text
>>> from example import add_to_sequence
>>> x = [1, 2, 3]
>>> add_to_sequence(x, 4)
Before: [1, 2, 3]
After: [1, 2, 3, 4]
>>> x
[1, 2, 3]
```

In the other direction we have the same problem:
when we reference a C++ `std::vector` instance
from within Python, any changes made to the Python instance
are not propagated to the C++ vector.
To demonstrate this, we define a global `std::vector<int>` variable in the C++
code, and a function to print this variable:

```c
std::vector<int> global_list {10, 11, 12};

void print_global_list()
{
    print_sequence(global_list);
}
```

In the mapping code we add the following to the module definition
to make this global variable
and the function available to Python:

```c
    m.attr("global_list") = &global_list;
    m.def("print_global_list", &print_global_list);
```

Note that we use the address of the `global_list`
in stead of the value of the variable.
Using a value would surely copy the contents before creating
a Python object, which would defeat the purpose.

We now see that we can access the global list in Python,
but that changes made in Python are not propagated to C++,
independent of whether they are made by Python code
or through a call the the `add_to_sequence` function in C++:

```text
>>> import conversion
>>> g = conversion.global_list
>>> g
[10, 11, 12]
>>> g.append(6)
>>> g
[10, 11, 12, 6]
>>> conversion.print_global_list()
[10, 11, 12]
>>> playground.add_to_sequence(g, 7)
Before: [10, 11, 12, 6]
After: [10, 11, 12, 6, 7]
>>> g
[10, 11, 12, 6]
>>> playground.print_global_list()
[10, 11, 12]
```

Since STL containers are very common,
there is often a need to manipulate them from
within Python as if they were native Python objects.
`pybind11` offers a binding that does that,
however at the cost of disabling the possibility of
using the conversion-by-copying method for these types.
To do that, the types we are interested in must be
made *opaque*.
In other words, they are the original C++ types
with a thin Python wrapper around them.
The `pybind11/stl_bind.h` header file contains
additional code that adds methods and properties to these opaque types
to mimic the behavior of the corresponding regular Python types.

```c++
PYBIND11_MAKE_OPAQUE(std::vector<int>);

PYBIND11_MODULE(conversion, m)
{
    ...
    py::bind_vector<std::vector<int>>(m, "VectorInt");

    m.def("add_to_sequence", &add_to_sequence<int>,
          py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    m.attr("global_list") = &global_list;
    m.def("print_global_list", &print_global_list);
    ...
}
```

Note that we have now introduced a new Python class `VectorInt`
that contains the wrapped `std::vector<int>` type.
With this change we can no longer use a regular Python list
to call e.g. `add_to_sequence`:

```text
>>> from conversion import add_to_sequence
>>> x = [1, 2, 3]
>>> add_to_sequence(x, 4)
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
TypeError: add_to_sequence(): incompatible function arguments. The following argument types are supported:
    1. (arg0: conversion.VectorInt, arg1: int) -> None

Invoked with: [1, 2, 3], 4
>>>
```

We must explicitly create a `VectorInt` instance to call this function.
When we do that, we see that the changes are indeed propagated:

```text
>>> from conversion import VectorInt
>>> x = VectorInt([1, 2, 3])
>>> x
VectorInt[1, 2, 3]
>>> x.append(4)
>>> x
VectorInt[1, 2, 3, 4]
>>> add_to_sequence(x, 5)
Before: [1, 2, 3, 4]
After: [1, 2, 3, 4, 5]
>>> x
VectorInt[1, 2, 3, 4, 5]
>>>
```

The other way around now also works:

```text
>>> g = conversion.global_list
>>> g
VectorInt[10, 11, 12]
>>> g.append(7)
>>> g
VectorInt[10, 11, 12, 7]
>>> conversion.print_global_list()
[10, 11, 12, 7]
>>>
```
