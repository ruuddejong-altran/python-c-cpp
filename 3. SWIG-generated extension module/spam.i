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
