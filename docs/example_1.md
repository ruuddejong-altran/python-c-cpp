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

```python
>>> import ctypes
>>> spam = ctypes.cdll.LoadLibrary('./spamlib')
>>>
```
Linux:

```python
>>> import ctypes
>>> spam = ctypes.cdll.LoadLibrary('./libspamlib.so')
>>>
```

You might be inclined now to look at the
contents of `spam`, and expect to see the functions `add`, `swap`, and `do_operation` listed.
But that method only shows standard Python magic methods and some implementation details from
`ctypes`, but not the symbols defined in `spamlib`.

```python
>>> dir(spam)
['_FuncPtr', '__class__', ..., '_handle', '_name']
>>>
```

The only way to find out if a function (or any other symbol) exists in the library,
is to try to use it.

```python
>>> spam.add(3, 5)
8
>>>
```

`ctypes` uses a caching mechanism for the symbols in the library, so from this point on the
`add` method is known:

```python
>>> dir(spam)
['_FuncPtr', '__class__', ..., '_handle', '_name', 'add']
>>>
```

Note that the `spam.add` function is *not*
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

```python
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

```python
>>> spam.add.argtypes = [ctypes.c_int, ctypes.c_int]
>>> spam.add.restype = ctypes.c_int
>>> spam.add(3, 5)
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
