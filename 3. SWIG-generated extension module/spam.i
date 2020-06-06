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

