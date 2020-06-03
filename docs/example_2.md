---
layout: default
title: Example 2
---

# Example 2 - Manually created extension module

In [example 1](./example_1.md) we saw a pure Python solution
for using a C-library.
This example shows a pure C solution for the same problem.
This solution creates a so-called *extension module*
for Python.

The reason that this is possible is that the Python import
mechanism does not only work for Python modules or packages.
You can also `import` shared libraries directly in Python.
The [Python documentation](https://docs.python.org/3/extending/index.html)
contains an extensive description of the process of creating
such an importable shared library.

## Creating the source file.

We cannot just use the `spamlib` shared library directly in Python code.
Just as in [example 1](./example_1.md), we must write some
wrapper code to create the bridge between Python and C.

By convention, the source code for a Python extension module `spam`
is called `spammodule.c`

## Debug versus release builds

One thing to be aware of is the difference between release and debug builds.
In general you will not want to use a debug version of the Python library -
after all, we are not going to debug the Python implementation.
Also, if you were to link to the debug version of the Python library,
you would also need to run a debug build of the Python interpreter to 
actually use the extension module. Otherwise the Python interpreter
will just crash when you try to import the extension module.

For these reasons we want
to use the optimized version of the Python library,
even for a debug build for our shared library.

The usual way to deal with this is to e.g. explicitly specify
an optimized library as link dependency for a debug build target.
But does not work in this case, because the Python header files,
at least for Windows builds, contain `#pragma` preprocessor statements
that force the use of the debug version of the Python library.

The recommended way to deal with this is to (temporarily) switch off the
DEBUG flag when including the Python header file.
The file `spammodule.c` therefore starts witt

```c
#define PY_SSIZE_T_CLEAN

#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif /* _DEBUG */

#include spamlib.h
```

It is recommended to always define `PY_SSIZE_T_CLEAN`.
This forces the use of the proper unsigned index type in stead of `int`
in some places in the Python library.
Its definition is checked within `Python.h`, so it must be defined
before `Python.h` is included.
As of Python 3.8, a `DeprecationWarning` will be raised if you did not
define it, and you use code that checks for this.

Finally, the line `#include spamlib.h` makes the public function definitions in `spamlib`
available in this extension module.

## Creating wrapper functions

The next step is to create the wrapper functions that will be called
by the Python code.
These wrapper functions must map the (Python) arguments in the function call
to the corresponding C parameters for the corresponding `spamlib` function.
Then they must call the `spamlib` function, and convert the result
back to a Python object before returning to the caller.

By convention the wrapper functions have names _\<module>\_\<function>_,
with _module_ the name of the extension module (`spam` in this case),
and _function_ the name of the function in the original library.

### Signature

All wrapper functions follow the same basic pattern:

```c
static PyObject *spam_<function>(PyObject *self, PyObject *args)
{
    ...    
}
```

For stand-alone functions like these,
the `self` argument points to the module object where the (Python) function lives.
In other words, if the Python code were to use `import spam` and then call `spam.add(3, 5)`,
the `self` argument would point to the module `spam`.

For instance methods, `self` would point to an actual Python object instance.

Since we don't use any module data in the implementation, the `self` argument
is not used in the function code.

### Argument parsing

The `args` argument points to an array of Python objects,
the actual (Python) arguments of the function call.
These must be converted into their C counterparts
in order to call the actual function from `spamlib`.
This conversion is done by:

```c
    if (!PyArg_ParseTuple(args, <format_string>, <pointer_to_variable>, ...))
    {
        return NULL;
    }
```

For each format specification in _format_string_,
a `PyObject` instance is taken from `args`,
converted into a C value according to the format specification,
and stored in the (next) _pointer_to_variable_.
If something goes wrong during this process,
a `NULL` pointer is returned,
which leads to a Python exception at the calling side.

### Building the return value

Building the result value is the mirror image of argument parsing.
Each result value is converted according to
the corresponding specification in _format_string_.

```c
    return Py_BuildValue(<format_string>, <result_value>, ...);
```

If only one value is converted, the result is just the Python
object that results from the conversion.
When there are multiple values, the result of
`Py_BuildValue` is a tuple with the corresponding Python objects.

### spam.add

The wrapper function for the `add` function is as straightforward as can be:

```c
static PyObject * spam_add(PyObject *self, PyObject *args)
{
    int x;
    int y;
    int result;

    /* Parse the python objects in args, and store them in the corresponding C variables */
    if (!PyArg_ParseTuple(args, "ii", &x, &y))
    {
        return NULL;
    }

    result = add(x, y);

    /* Transform the result into a Python int object, and return that. */
    return Py_BuildValue("i", result);
}
```

### spam.swap

The wrapper function for the `swap` function is almost as straightforward.
The thing to note here is that the function returns a tuple with
the swapped values.

```c
/* Wrapper function for the swap function in spamlib */
static PyObject * spam_swap(PyObject *self, PyObject *args)
{
    int x;
    int y;

    /* Parse the python objects in args, and store them in the corresponding C variables */
    if (!PyArg_ParseTuple(args, "ii", &x, &y))
    {
        return NULL;
    }

    swap(&x, &y);
    /*
     * Transform the result into a Python tuple, and return that.
     * Note that Python cannot do an inplace replacement of a variable.
     */
    return Py_BuildValue("ii", x, y);
}

```
### spam.do_operation

The code for `do_operation` is more complex.

First of all, the C code must call back to the Python function `operator`.
That means that we must have a function that is acceptable
for the `spamlib.do_operation` function, but that internally calls the
supplied Python function:

```c
static PyObject *py_callback_func = NULL;

static int operator_wrapper_func(int x, int y)
{
    int retval = 0;
    
    /*
     * Ensure that the callback function stays alive during the call to do_operation.
     * This is not guaranteed, because the callback is executed in Python,
     * and in principle the callback function could be deleted during that call.
     * Incrementing the refcount ensures that the reference will remain valid.
     */
    Py_INCREF(py_callback_func);

    /* Call the Python callback function. */
    PyObject *result = PyObject_CallFunction(py_callback_func, "ii", x, y);
    if (result && PyLong_Check(result))
    {
        retval = PyLong_AsLong(result);
    }

    /* Prevent memory leaks! */
    Py_DECREF(py_callback_func);
    Py_XDECREF(result);
    return retval;
}
```

The actual Python function is stored in the global (yuck) variable `py_callback_func`.
We call back into the Python code with the `PyObject_CallFunction` call.
This takes the callback function, a format string, and a series of arguments.
These latter should be familiar by now.

The Python object that is returned is validated, converted to a C long,
and returned to the caller.

Please note the `Py_INCREF`, `Py_DECREF`, and `Py_XDECREF` calls.
Python objects dynamically allocated.
That means that their storage space must be released
when they are no longer relevant.
Python uses reference counting for heap-allocated objects
(which means: all objects).
The reference count is an attribute of the `PyObject` structure.
The memory allocated for an object is freed when its
reference count goes to zero.

The reference count for objects that are passed into a function as arguments
(such as the callback function that is stored in `py_callback_func`)
is not incremented by the Python machinery.
The calling function has a reference to these objects,
and since the calling function suspends execution
until the called function is finished,
that reference will remain valid during the execution
of the called function.

But now we call back from C back into Python.
And in principle anything can happen there.
Including reaching back from the callback function
to the caller, and deleting the callback function.
To safeguard against that (admittedly remote) possibility,
the reference count of the callback function
is temporarily incremented,
and decremented again after callback function has been called.

The Python object `result` that we get back from the call to `PyObject_CallFunction`
is a so-called _new_ _reference_, with a reference count of 1.
That means that this function owns the only reference to that object.

If we were to return without decrementing that reference count,
the object would still exist on the heap with
a reference count of 1.
But our `result` local variable would be out of scope,
and there would no references left that point to this object.
In other words, we would have created a memory leak.

By calling `Py_XDECREF`, we decrement the reference count of the object
so that its memory can be freed.
`Py_XDECREF` differs from `Py_DECREF` in that it accepts `NULL` pointers.

Having set the stage for the callback function, the actual wrapper function
for `do_operation` is again straightforward.
The Python callback function is stored in `py_callback_func`,
and the do_operation function is called with the
`operator_wrapper_func` as callback function:

```c
/* Wrapper function for the do_operation function in spamlib */
static PyObject * spam_do_operation(PyObject *self, PyObject *args)
{
    int x;
    int y;

    /* Parse the python objects in args, and store them in the corresponding C variables */
    if (!PyArg_ParseTuple(args, "iiO", &x, &y, &py_callback_func))
    {
        return NULL;
    }

    /* Ensure that the Python callback is callable */
    if (!PyCallable_Check(py_callback_func))
    {
        return NULL;
    }

    int result = do_operation(x, y, &operator_wrapper_func);
    return Py_BuildValue("i", result);
}
```

## Making this a Python extension module

Finally there are some bookkeeping chores that are required to make
this a proper Python module.

First the functions that are offered by this method must be declared.
For this we specify their names and addresses in a _method table_.
This table also contains the way arguments are passed to the function, and optionally its docstrings.
The `METH_VARARGS` flag indicates that the function expects a tuple of positional arguments.
If we would also require keyword arguments, then we would add the `METH_KEYWORDS`
flag, like `METH_VARARGS | METH_KEYWORDS`.

```c
/* The methods of the module.*/
static PyMethodDef spam_methods[] = {
        {"add", spam_add, METH_VARARGS, "Add two numbers."},
        {"swap", spam_swap, METH_VARARGS, "Swap two values."},
        { "do_operation", spam_do_operation, METH_VARARGS, "Perform operation on two numbers."},
        {NULL, NULL, 0, NULL}  /* Sentinel */
};
```

The next step is to define the module itself.
This is done in a _module definition structure_:

```c
/* The actual definition of the module */
static struct PyModuleDef spammodule = {
        PyModuleDef_HEAD_INIT,
        "spam",  /* name of the module */
        NULL,  /* module documentation, may be NULL */
        -1,  /* size of per-interpreter state of the module, or -1 if module keeps state in global variables. */
        spam_methods
};
```

And as a last step the module's initialization function must be defined.
This function is executed when the module is imported in Python.
Note that this should be the _only_ non-static item in the file.

```c
/* This is the initialization function that is called when the module is loaded */
PyMODINIT_FUNC PyInit_spam(void)
{
    return PyModule_Create(&spammodule);
}
```

And that's it. Except for the minor but oh-so-frustating-if-forgotten detail for Windows:
the compiled library's extension must be `.pyd` in stead of `.dll`.
In the example directory I have included a `CMakeLists.txt` file that automatically
takes care of this.

## Discussion

As with the pure Python example in [example 1](./example_1.md),
making a Python extension module in C requires a lot of boiler plate code.
Perhaps even more so than the pure Python solution.
Especially keeping track of the reference counts of Python objects
is often tricky.

An additional disadvantage of this solution is that it is not thread-safe.
There is currently no lock around the global variable `py_callback_func`,
which means that it can be overwritten by another thread before
the actual call to `spamlib.do_operation` is executed.
This can of course be resolved by protecting that global variable
with a mutex. I have not done so in this example, to avoid adding
additional clutter that is not essential to the topic at hand.
