---
layout: default
title: Example 1
---

# Example 1 - Using ctypes

The most basic way to use a C or C++ library
is with the `ctypes` module from
Python's standard library.
It is convenient to `cd` first to the directory
that contains the `spamlib` library.
The reason for that is that `ctypes` requires the path
to the library, and the directory name
for the build results varies between systems
and IDEs.

## Making the library available

When you are in the correct library,
you can start Python there.
Python always includes the current directory
in its search path, so the `spamlib` library
can be loaded with `ctypes`, using the relative
path name to the library.
Note that on Windows, the library extension (`.dll`)
is not necessary, but on Linux the complete
file name is required.

Windows:

```
>>> import ctypes
>>> spam = ctypes.cdll.LoadLibrary('./spamlib')
>>>
```
Linux:

```
>>> import ctypes
>>> spam = ctypes.cdll.LoadLibrary('./libspamlib.so')
>>>
```

You might be inclined now to look at the
contents of `spam`, and expect to see the functions `add`, `swap`, and `do_operation` listed.
But that method only shows standard Python magic methods and some implementation details from
`ctypes`, but not the symbols defined in `spamlib`.

```
>>> dir(spam)
['_FuncPtr', '__class__', ..., '_handle', '_name']
>>>
```

The only way to find out if a function (or any other symbol) exists in the library,
is to try to access it.

```
>>> spam.add
<_FuncPtr object at 0x00000246D65E22B0>
>>>
```

`ctypes` uses a caching mechanism for the symbols in the library, so from this point on the
`add` method is known:

```
>>> dir(spam)
['_FuncPtr', '__class__', ..., '_handle', '_name', 'add']
>>>
```

## spam.add

Now that we know that the function `add` is indeed present in the library,
we can of course see if we can use it:

```
>>> x = 3
>>> y = 5
>>> spam.add(x, y)
8
>>>
```

The `spam.add` function is *not*
the real C function from the library.
Instead it is a (thin) wrapper around this library function.
This wrapper function, amongst other things, converts the Python integer
objects `3` and `5` in the function call to their C counterparts,
and converts the C `int` result back into a Python integer object
that is returned to the caller.
`ctypes` converts Python `int`s, `byte` objects and strings automatically,
and expects by default an integer result value from the C function.
That is why the `add` function could be called with `spam.add(3, 5)`,
and why the result is represented as a Python integer.

The creation of a wrapper function takes time, and is therefore
only done when needed.
That is the reason that the symbols do not initially appear
in the contents of `spam`.

Note that there is very little checking done on the types and number
of the arguments:

```
>>> spam.add('abc', 'def')
650157296
>>> spam.add(3.45, 5,67)
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
ctypes.ArgumentError: argument 1: <class 'TypeError'>: Don't know how to convert parameter 1
>>> spam.add(1, 2, 3)
3
>>> spam.add(1)
2
>>> spam.add()
-1929449448
>>> spam.add(1, 'abc')
-1822405007
>>> spam.add('abc', 2)
-1822404974
>>>
```

The reason that Python strings are accepted as arguments, but floats not,
is (probably) that `ctypes` maps a Python `str` to a C `wchar_t` pointer,
which in turn can be converted to a C integer.

You can make using a C library via `ctypes` more robust
by specifying the function prototype,
in effect repeating in Python what is already specified in the `spamlib.h` header file.

```
>>> spam.add.argtypes = [ctypes.c_int, ctypes.c_int]
>>> spam.add.restype = ctypes.c_int
>>> spam.add(x, y)
8
>>> spam.add('abc', 'def')
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
ctypes.ArgumentError: argument 1: <class 'TypeError'>: wrong type
>>> spam.add(3)
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
TypeError: this function takes at least 2 arguments (1 given)
>>>
```

## spam.swap

Using the `swap` function requires a little more work.
First we define the function prototype in Python:

```
>>> spam.swap.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int)]
>>> spam.swap.restype = None
>>>
```

As an aside: just referencing the function (`spam.swap`) make `ctypes` create the
wrapper function .
The `.argtype` and `.restype` attributes therefore automatically
refer to the Python wrapper function.

`ctypes` has no possibility to generate a pointer to a Python object.
To actually use the `swap` function we must therefore first create
`ctypes.c_int` objects from the original Python `int` objects.
After calling `swap` with pointers to these objects,
we can assign their Python values back to the original Python variables.

```
>>> _x = ctypes.c_int(x)
>>> _y = ctypes.c_int(y)
>>> spam.swap(ctypes.pointer(_x), ctypes.pointer(_y))
>>> x = _x.value
>>> y = _y.value
>>> print(f'x: {x}, y: {y}')
x: 5, y: 3
>>>
```

## spam.do_operation

Finally, the `do_operation` function requires a callback function.
We start by defining the signatures of both the callback function
and the `do_operation` function itself.
The callback function signature is defined using
`ctypes.CFUNCTYPE`, which expects the return type as first argument,
followed by the types of the function arguments.
This gives a Python class that can be used both in the `.argtypes`
specification for the `do_operation` function,
and for casting a Python function into the expected format
for a call to the C-library.

```
>>> operator_functype = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_int, ctypes.c_int)
>>> spam.do_operation.argtypes = [ctypes.c_int, ctypes.c_int, operator_functype]
>>> spam.do_operation.restype = ctypes.c_int
```

To use the `do_operator` function, we need a Python function
that can act as callback function.
We need to define that first.
The `print` function provides evidence that we are indeed calling the Python function
from the C library.

```
>>> def subtract(x, y):
...     print(f'Python subtract function called with x={x}, y={y}')
...     return x - y
...
>>>
```

We can now call `do_operation` with this callback function
(remember to cast it to the expected type):

```
>>> spam.do_operation(x, y, operator_functype(subtract))
Python subtract function called with x=5, y=3
2
>>>
```

