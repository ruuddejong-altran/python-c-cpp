---
layout: default
title: Example 3
---

# Example 3 - SWIG-generated extension module

In [example 1](./example_1.md) we saw a pure Python solution
for using a C-library,
and in [example 2](./example_2.md) a manually created extension module.
Both methods require a lot of boilerplate code, and require keeping
track of reference counts.

Fortunately there are tools that can do this for us.
[Swig](https://swig.org) (Simplified Wrapper and Interface Generator)
is one such tool that is quite often used
for exactly this purpose.
In fact, it can build wrappers for many more languages than just Python. 

SWIG works through so-called _interface files_, conventionally with a `.i` extension.
An interface file has a few parts:

 * Specification of the wrapper's name via the `%module` directive
 * Declaration of external code used by the wrapper.
   This is C or C++ code between a pair of `%{`, `%}`
   directives that is added to the generated wrapper source file,
   in order to make it compilable.
   Typically this would be a `#include` statement for thw wrapped library's header file.
 * Declaration of the symbols that must be made available by the wrapper.
   Typically this would again be the contents of the library's header file.
   As we will see though, there are situations that require some fine tuning.

I slightly rewrote the `sliplib` source file,
in order to use C++ code.
It now looks like:

```c

```

A first attempt at the interface file would e.g. be a file `spam.i` with contents:

```
%module spam

%begin %{
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
%}

%{
/* Include header file in generated wrapper code */
#include "spamlib.h"
%}

/* Parse header file to generate wrappers */
%include "spamlib.h"
```

The line with `SWIG_PYTHON_INTERPRETER_NO_DEBUG` serves to avoid the problem
with the linker attempting to link to the debug version of the Python libraries,
see [example 2](./example_2.md).
When we run `swig -c++ -python spam.i`,
this causes the following code fragment to be included in the
generated wrapper source file:

```c
#if defined(_DEBUG) && defined(SWIG_PYTHON_INTERPRETER_NO_DEBUG)
/* Use debug wrappers with the Python release dll */
# undef _DEBUG
# include <Python.h>
# define _DEBUG 1
#else
# include <Python.h>
#endif
```

When we compile and link this wrapper source file
we get a `spam.py` module that can be imported in Python,
and works, at least for the `add` function:

```
>>> import spam
>>> spam.add(3, 5)
8
```

But the `swap` and `do_operation` functions do not work.
Trying `swap` we get:

```
>>> x = 3
>>> y = 5
>>> spam.swap(x, y)
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
  File "D:\GitHub\python-c-cpp\3. SWIG-generated extension module\cmake-build-debug\spam.py", line 69, in swap
    return _spam.swap(a, b)
TypeError: in method 'swap', argument 1 of type 'int &'
>>> 
```

And trying `do_operation` we get:

```
>>> def subtract(x, y):
...     print(f'Python subtract({x}, {y})')
...     return x - y
...
>>> spam.do_operation(5, 3, subtract)
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
  File "D:\GitHub\python-c-cpp\3. SWIG-generated extension module\cmake-build-debug\spam.py", line 72, in do_operation
    return _spam.do_operation(a, b, operator_func)
TypeError: in method 'do_operation', argument 3 of type 'std::function< int (int,int) >'
>>>
```

The problem with `swap` is understandable.
SWIG only looks at the function signatures,
and cannot know that the parameters
are used for both input and output.
We need to provide additional information to SWIG
in order to handle that.

Fortunately SWIG provides some magic directives to handle this
(and many more) situations where you need special treatment
of arguments.
The solution for the problem with `swap` is to include the directive `%include "typemaps.i"`
at the beginning of the interface file.
This makes it possible to apply (amongst other things)
`INPUT`, `OUTPUT`, or `INOUT` mapping to function parameters.
The only drawback is that we can no longer simply `%include spamlib.h`;
instead we have to specify the function signatures by hand.

```
%module spam

%include "typemaps.i"

%begin %{
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
%}

%{
/* Include header file in generated wrapper code */
#include "spamlib.h"
%}

/* Specify the Python-callable wrappers */
extern int add(int, int);
extern void swap(int& INOUT, int& INOUT);
extern void do_operation(int, int, int(*)(int, int));
```

After rerunning the swig command and compiling the resulting wrapper file,
we can see that it now works:

```
>>> import spam
>>> x = 3
>>> y = 5
>>> x, y = spam.swap(x, y)
>>> x
5
>>> y
3
>>>
```

That leaves the problem with the callback function.