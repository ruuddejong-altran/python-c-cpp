%module spam

%include "typemaps.i"

%begin %{
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
%}

%{
/* Include header file in generated wrapper code */
#include "traffic_light.h"

%}

//%include "clsTrafficLight.h"
