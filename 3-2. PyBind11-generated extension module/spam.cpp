#include "pybind11/pybind11.h"
#include "pybind11/functional.h"

#include "spamlib.h"

namespace py = pybind11;

using namespace py::literals;

PYBIND11_MODULE(spam, m)
{
    m.doc() = "Example extension module";
    m.def("add", &add, "Add two integers", "x"_a, "y"_a);
    m.def("swap", [](int x, int y) { swap(x, y); return std::make_tuple(x, y); },
          "Swap two values", "x"_a, "y"_a);
    m.def("do_operation", &do_operation, "Perform operation on two integers",
          "x"_a, "y"_a, "operation"_a);
}
