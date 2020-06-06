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
[SWIG](https://swig.org) (Simplified Wrapper and Interface Generator)
is one such tool that is quite often used
for exactly this purpose.
In fact, it can build wrappers for many more languages than just Python.

SWIG creates a hybrid solution: it generates a `spam.py` file that contains
the Python-language definitions required for the interface,
and a `_spam.pyd` or `_spam.so` shared
library that contains the C or C++ implementation for the interface.

SWIG works through so-called _interface files_, conventionally with a `.i` extension.
An interface file has a few parts. Minimally needed are:

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
```

## First attempt

A first attempt at the interface file would e.g. be a file `spam.i`.

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
we get a `spam.py` module that can be imported in Python.
We can see the `add` function works:

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

## Solving swap

The problem with `swap` is understandable.
SWIG only looks at the function signatures,
and cannot know that the parameters
are used for both input and output.
We need to provide additional information to SWIG
in order to handle that.

Fortunately SWIG provides some magic directives to handle this
(and many more) situations where you need special treatment
of arguments and argument types.
The solution for the problem with `swap` is to include the directive `%include "typemaps.i"`
at the beginning of the interface file.
This makes it possible to apply (amongst other things) a mapping
(`INPUT`, `OUTPUT`, or `INOUT`) to function parameters.
Parameters mapped with `OUTPUT` or `INOUT` get collected in a tuple
that forms the return value of the wrapped function.

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
we can see that `swap` now works:

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

## Solving do_operation

That leaves the problem with the callback function.
That, unfortunately, takes some more effort.
SWIG has a _director_ feature that allows calling back
from C++ to Python.
But this requires
that the Python callback code is an overriden virtual method
in a Python class that is derived from a C++ class.

So, in order to make this work we must:

1. Define a C++ class with a virtual method,
2. Derive a Python class from this C++ class,
   and override the virtual method to call the function that we want to call.

Now, we don't want to touch the original C++ library code -- it could be that
we don't even have access to it.
And we also don't want to force the Python users of the wrapped code to
go to the overhead of deriving a subclass from a C++ class that did not
even exist in the original library.

Fortunately, we can simply include C++ and Python code in the interface file.
What's more, we can also instruct SWIG to parse the C++ code
we add, and generate Python wrappers for it, just as it would
do for the original library code.

First we must enable the director feature.
To that end we modify the first line of the `spam.i` interface file:

```
%module(directors="1") spam
``` 

The next step is to define the C++ class with the virtual method.
The `%feature(director)` line tells SWIG to generate code
that allows C++ to call code defined in a Python subclass of this class.
And the `%inline` directive tells SWIG to not only include the code
in the generated C++ wrapper file,
but to also generate this class in the Python file.
In line with the Python convention,
the name of the class starts with an underscore,
to indicate that it is an implementation detail, and not part of the formal interface.

```
%feature("director") _OperationFuncClass;

%inline %{
struct _OperationFuncClass {
    virtual int operation_method(int a, int b) = 0;
    virtual ~_OperationFuncClass() {}
};
%}
```

Now that we have this class, we need to add some Python code
to the `spam.i` interface file.
In the implementation of the `do_operation` function
we define a (Python) subclass of `_OperatorFuncClass`,
with the virtual method `operation_method` redefined to
execute the `operation_func` argument.
This class is then handed over to a (yet to be defined) C++ function
`_operation_wrapper`
that will call the virtual method.

We now define the `do_operation` Python function that we want to
be included in the generated `spam.py` file.
```
%pythoncode
%{
def do_operation(x, y, operation_func):
    class PythonOperation(_OperationFuncClass):
        def operation_method(self, a, b):
            return operation_func(a, b)
    python_operation_object = PythonOperation()
    return _operation_wrapper(x, y, python_operation_object)
%}
```

In the function body we define a class
that is derived from `_OperatorFuncClass`,
with a redefined `operation_method`.
We create an instance of that class,
and we use that instance as the third argument
to the `_operation_wrapper` function.

The `%pythoncode` directive tells SWIG to include this
code in the generated `spam.py` file.
No code occurs in the generated `.cpp` file for this.

Now all that is needed to finish this
is to create the `_operation_wrapper` function.
This function must be available both in C++ and Python, so we use
the `%inline` directive again.
But since the `do_operation` function in the library expects
a plain function as its 3rd argument, and not a method,
we must again (similar to what we did in [example 2](./example_2.md))
create a helper function that can be passed as the third argument.
The helper function, as well as the global variable
used to store the pointer to the `_OperationFuncClass` instance,
are relevant only in the C++ code.
Therefore that part does not have the `%inline` directive.

```
%{
static _OperationFuncClass *operation_func_class_ptr = NULL;

static int operation_helper(int a, int b) {
    return operation_func_class_ptr->operation_method(a, b);
}
%}

%inline %{
int _operation_wrapper(int a, int b, _OperationFuncClass *operation_func_class) {
    operation_func_class_ptr = operation_func_class;
    int result = do_operation(a, b, &operation_helper);
    operation_func_class_ptr = NULL;
    return result;
}
%}
```

With these additions to the interface file,
we can rebuild the project
We see that now everything works.

```
>>> import spam
>>> x = 3
>>> y = 5
>>> spam.add(x, y)
8
>>> x, y = spam.swap(x, y)
>>> print(x, y)
5 3
>>> def subtract(a, b):
...     return a - b
...
>>> spam.do_operation(x, y, subtract)
2
>>>
```

# Discussion

To make this example work with SWIG, we had to provide an interface file `spam.i`
that looks like:

```
%module(directors="1") spam

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

%feature("director") _OperationFuncClass;

%inline %{
struct _OperationFuncClass {
    virtual int operation_method(int a, int b) = 0;
    virtual ~_OperationFuncClass() {}
};
%}

%{
static _OperationFuncClass *operation_func_class_ptr = NULL;

static int operation_helper(int a, int b) {
    return operation_func_class_ptr->operation_method(a, b);
}
%}

%inline %{
int _operation_wrapper(int a, int b, _OperationFuncClass *operation_func_class) {
    operation_func_class_ptr = operation_func_class;
    int result = do_operation(a, b, &operation_helper);
    operation_func_class_ptr = NULL;
    return result;
}
%}

%pythoncode
%{
def do_operation(x, y, operation_func):
    class PythonOperation(_OperationFuncClass):
        def operation_method(self, a, b):
            return operation_func(a, b)
    return _operation_wrapper(x, y, PythonOperation())
%}
```

This is quite some programming.
But much of that is caused by the fact that we want to use
a Python function as callback from a plain C++ function
that expects a plain function as callback.
In most real C++ applications the callback function
would probably be a method,
and the entity accepting the callback would most likely
be a class instance.

For real-life C++ applications we would therefore probably already
have C++ classes that we can inherit from,
so we would not have to create our own.
And if we would use a method call for the callbacks,
not a function call,
then that would remove the need for the global variable
and the helper function.
In other words, the ratio of generated-interface-code to manually-written-interface-code
gets better when we start using actual C++ classes.
We will see this in the upcoming examples.

Note that, compared with [example 2](./example_2.md),
we don't do any increments or decrements of the reference counts
for the Python objects.
All that bookkeeping is taken care of by the generated C++ wrapper code.

You might think that, just as with [example 2](./example_2.md),
that this code is not thread-safe.
Actually it is, but that is because SWIG by default disables
support for multi-threaded Python applications.
That means that while the wrapped C++ code is running,
the Python interpreter will not be able to run any other threads.
You can use SWIG directives to allow multi-threading
for a wrapped module,
and you can even enable or disable thread support for specific methods.