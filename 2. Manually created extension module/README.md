This example directory again contains the source code for the C library `spamlib`
with the three utiltiy functions: `add`, `swap`, and `do_operation`.
This is the same as in the `1. Basic C library usage` example.

`demo.py` now simply uses an `import spam` statement to access this library.

To make that possible, a C-level extension `spammodule.c` contains all the
boilerplate code that is needed to make this work. And again, this is barely
scratching the surface,
because in real-life far more checks and error handling would be needed.

Also note the `CMakeLists.txt` file, which now contains code to detect the Python
include paths and libraries, so that the actual extension module compilation and linkage
is done automatically, and results in a library with the correct extension (`.pyd` instead
of `.dll` or `.so`)