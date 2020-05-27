#define PY_SSIZE_T_CLEAN

/*
 * The following is necessary to avoid linking
 * to the debug Python library.
 */
#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif /* _DEBUG */

#include "spamlib.h"

/* Wrapper function for the add function in spamlib */
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


/*
 * Variable to store the Python callback function.
 * Since standard C does not allow nested functions,
 * portability requires this mechanism.
 */
static PyObject *py_callback_func = NULL;

/*
 * Wrapper for the Python callback function.
 * This is the function that is actually offered to the
 * do_operation function in spamlib.
 */
static int callback_wrapper_func(int x, int y)
{
    int retval = 0;

    /* Call the Python callback function. */
    PyObject *result = PyObject_CallFunction(py_callback_func, "ii", x, y);
    if (result && PyLong_Check(result))
    {
        retval = PyLong_AsLong(result);
    }
    /* Prevent memory leaks! */
    Py_XDECREF(result);
    return retval;
}

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

    /*
     * Ensure that the callback function stays alive during
     * the call to do_operation.
     * This is not guaranteed, because the callback is
     * executed in Python, and in principle the callback
     * function could be deleted during that call.
     * Incrementing the refcount ensures that the reference will
     * remain valid.
     */
    Py_INCREF(py_callback_func);
    int result = do_operation(x, y, &callback_wrapper_func);
    /* Prevent memory leaks! */
    Py_DECREF(py_callback_func);

    return Py_BuildValue("i", result);
}

/* The methods of the module.*/
static PyMethodDef spam_methods[] = {
        {"add", spam_add, METH_VARARGS, "Add two numbers."},
        {"swap", spam_swap, METH_VARARGS, "Swap two values."},
        { "do_operation", spam_do_operation, METH_VARARGS, "Perform operation on two numbers."},
        {NULL, NULL, 0, NULL}  /* Sentinel */
};

/* The actual definition of the module */
static struct PyModuleDef spammodule = {
        PyModuleDef_HEAD_INIT,
        "spam",  /* name of the module */
        NULL,  /* module documentation, may be NULL */
        -1,  /* size of per-interpreter state of the module, or -1 if module keeps state in global variables. */
        spam_methods
};

/* This is the initialization function that is called when the module is loaded */
PyMODINIT_FUNC PyInit_spam(void)
{
    return PyModule_Create(&spammodule);
}
