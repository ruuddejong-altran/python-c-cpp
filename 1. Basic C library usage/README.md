This example directory contains the source code for the C library `spamlib`.
This library contains three utiltiy functions: `add`, `swap`, and `do_operation`.

The file `main.c` contains a C program that uses this library,
and the file `demo.py` contains a Python program that also uses
this library, through the `ctypes` module.

`demo.py` illustrates the Python boilerplate code that is necessary to use
the functions offered by `spamlib`. And even this is only scratching the surface,
because a production-ready application would require far more checks and error handling.