---
layout: default
title: Example 3-2
---

# Example 3-2 - pybind11-generated extension module

In the [previous example](./example_3-1.md) we saw how SWIG
can be used to generate an extension module.
We saw that using SWIG requires learning a new language
(for the interface specification file),
and requires still a lot of manual coding for the callback function.

An alternative wrapper generator is `pybind11`.
As the name suggests, this can generate Python wrappers only,
and requires that the wrapped language is at least C++11.
It is therefore much more limited than SWIG.

`pybind11` is not a tool that can be installed.
Instead it is essentially a collection of header files.
The recommended way to use `pybind11` is to include its
repository in a Git submodule of your own project.
You can then use the extra CMake functions defined
by `pybind11` to generate the wrapper.
In this example, essentially only the following additional lines are required
in CMakeLists.txt to generate the extension module:

```
add_subdirectory(pybind11)
pybind11_add_module(spam MODULE spam.cpp)
target_link_libraries(spam PRIVATE spamlib)
```

## Specifying the wrapper code

Since `pybind11` is basically a collection of C++ header files,
the actual Python module definition is written in a C++ file.
In this case the module definition is contained in a file `spam.cpp`.
The first attempt is a vary minimal one, following the guidelines
in the `pybind11` documentation.

```c
#include "pybind11/pybind11.h"

#include "spamlib.h"

PYBIND11_MODULE(spam, m)
{
    m.def("add", &add);
    m.def("swap", [](int x, int y) { swap(x, y); return std::make_tuple(x, y); });
    m.def("do_operation", &do_operation);
}
```

The two `#include` statements are necessary to have the `pybind11` and the `spamlib`
declarations available.
Next is the module definition that uses the `PYBIND11_MODULE` macro.
Note the similarity with the manually code module definition
in [example 2](./example_2.md).

The `swap` function from `spamlib` is actually the only
function that is wrapped, so that it can return
a result tuple.
The functions `add` and `do_operation` are simply delegated
directly to their `spamlib` implementations.

This compiles without any problem.
Which is actually quite surprising, because for both
the manually created extension module in [example 2](./example_2.md)
and the SWIG-created extension module in [example 3-1](./example_3-1.md)
we had to do something special in order to use the
optimized Python library for debug builds.

Let us now see what we have got.
Starting Python in the build directory, we can import `spam`
and look at its contents.

```
>>> import spam
>>> dir(spam)
['__doc__', '__file__', '__loader__', '__name__', '__package__', '__spec__', 'add', 'do_operation', 'swap']
>>>
```

We see that `spam` offers a nice, clean interface, at least compared to the
interface offered by the SWIG-generated extension module.

Let's see if the functionality is there.
The functions `add` and `swap` actually work as expected,
but the `do_operation` does not work yet.
As before in the other examples, it complains about a mismatch
between the types of the Python function and the `std::function<int(int, int)>`
expected by `spamlib.do_operation`.

```
>>> spam.add(3, 5)
8
>>> x = 3
>>> y = 5
>>> x, y = spam.swap(x, y)
>>> x, y
(5, 3)
>>> def subtract(x, y):
...     return x - y
...
>>> spam.do_operation(x, y, subtract)
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
TypeError: do_operation(): incompatible function arguments. The following argument types are supported:
    1. (arg0: int, arg1: int, arg2: std::function<int __cdecl(int,int)>) -> int

Invoked with: 5, 3, <function subtract at 0x000001316162DAF0>

Did you forget to `#include <pybind11/stl.h>`? Or <pybind11/complex.h>,
<pybind11/functional.h>, <pybind11/chrono.h>, etc. Some automatic
conversions are optional and require extra headers to be included
when compiling your pybind11 module.
>>>
```

But note that, unlike in the previous cases, a helpful message
is displayed with suggestions on how to resolve the problem.
Looking at this message and at the `pybind11` documentation,
it would appear that including `pybind11/functional.h`
in the source file might just solve this.

```c
#include "pybind11/pybind11.h"
#include "pybind11/functional.h"

#include "spamlib.h"

PYBIND11_MODULE(spam, m)
{
    m.def("add", &add);
    m.def("swap", [](int x, int y) { swap(x, y); return std::make_tuple(x, y); });
    m.def("do_operation", &do_operation);
}
```

When we build this, and again open the Python interpreter
in the build directory, we see that it now works:

```
>>> import spam
>>> def subtract(x, y):
...     return x - y
...
>>> spam.do_operation(5, 3, subtract)
2
>>>
```

So that's it.
With just 9 lines of C++ code, we have created
a wrapper for our library.

`pybind11` offers some additional capabilities that can make life
for both C++ and Python developers easier.
One of these are docstrings and named arguments.
When we run `help(spam.do_operation)` in the Python interpreter
we get a message that essentially shows the signature of the function:

```
>>> help(spam.do_operation)
Help on built-in function do_operation in module spam:

do_operation(...) method of builtins.PyCapsule instance
    do_operation(arg0: int, arg1: int, arg2: Callable[[int, int], int]) -> int

>>>
```

This is already far more helpful than what we get for the SWIG-generated module.
But we can make this even better.
`pybind11` gives an easy way to provide docstrings to modules and functions,
and to provide names to the arguments.
The `.def()` method of a module takes additional arguments
that specify the docstring and the arguments for a function.
Using this, our `spam.cpp` file becomes:

```
#include "pybind11/pybind11.h"
#include "pybind11/functional.h"

#include "spamlib.h"

namespace py = pybind11;

PYBIND11_MODULE(spam, m)
{
    m.doc() = "Example extension module";
    m.def("add", &add, py::arg("x"), py::arg("y"), "Add two integers");
    m.def("swap", [](int x, int y) { swap(x, y); return std::make_tuple(x, y); },
          py::arg("x"), py::arg("y"), "Swap two values" );
    m.def("do_operation", &do_operation, py::arg("x"), py::arg("y"), py::arg("operation"),
          "Perform operation on two integers");
}
```

After compiling we can run Python in the build directory,
and we get the following when we call help() on the spam module:

```
>>> import spam
>>> help(spam)
Help on module spam:

NAME
    spam - Example extension module

FUNCTIONS
    add(...) method of builtins.PyCapsule instance
        add(x: int, y: int) -> int

        Add two integers

    do_operation(...) method of builtins.PyCapsule instance
        do_operation(x: int, y: int, operation: Callable[[int, int], int]) -> int

        Perform operation on two integers

    swap(...) method of builtins.PyCapsule instance
        swap(x: int, y: int) -> Tuple[int, int]

        Swap two values

FILE
    d:\github\python-c-cpp\3-2. pybind11-generated extension module\cmake-build-debug\spam.cp38-win_amd64.pyd


>>>
```

And because writing `py::arg()` for each argument name is tiresome,
`pybind11` offers a string literal operator `_a`,
that calls `py::arg()` on the string.
To use that we must add a `using namespace pybind11::literals`
to make this operator available:

```c
#include "pybind11/pybind11.h"
#include "pybind11/functional.h"

#include "spamlib.h"

namespace py = pybind11;

using namespace py::literals;

PYBIND11_MODULE(spam, m)
{
    m.doc() = "Example extension module";
    m.def("add", &add, "Add two integers", "x"_a, "y"_a);
    m.def("swap", [](int x, int y) { swap(x, y); return std::make_tuple(x, y); },
          "Swap two values", "x"_a, "y"_a);
    m.def("do_operation", &do_operation, "Perform operation on two integers",
          "x"_a, "y"_a, "operation"_a);
}
```

## Discussion

`pybind11` gives the possibility to create Python wrappers for C++ code
that is compiled with C++11 or later.
It requires a minimal amount of user-defined code to get a usable result.
So why would you not always use this in stead of SWIG?

It depends on your audience.
If you have a C++ library that you want to make available to
many other languages, such as Lua, Ruby, Perl, and so on,
then SWIG is probably your best choice.
Also if you have C++ code that is pre-C++11,
or even plain old C code,
you're better of with SWIG than with `pybind11`.

But if you just need to have Python and C++ interworking
in your project,
and are not interested in other languages,
then `pybind11` might be the better choice.
It does not require a tool installation and it
does not force you to learn a new language.
And the amount of wrapping code you need is very minimal.
