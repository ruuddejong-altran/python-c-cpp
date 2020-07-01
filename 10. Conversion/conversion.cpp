#include "pybind11/pybind11.h"
#include "pybind11/functional.h"
#include "pybind11/iostream.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

#include "example.h"


namespace py = pybind11;

using namespace py::literals;

PYBIND11_MAKE_OPAQUE(std::vector<int>);

PYBIND11_MODULE(conversion, m)
{
    m.doc() = "Conversion examples";

    py::bind_vector<std::vector<int>>(m, "VectorInt");

    m.def("add_to_sequence", &add_to_sequence<int>,
          py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    m.attr("global_list") = &global_list;
    m.def("print_global_list", &print_global_list);
}
