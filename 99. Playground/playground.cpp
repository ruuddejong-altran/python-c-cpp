#include "pybind11/pybind11.h"
#include "pybind11/functional.h"
#include "pybind11/iostream.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

#include "playgroundlib.h"


namespace py = pybind11;

using namespace py::literals;

//void del_py_object(PyObject* obj)
//{
//    Py_XDECREF(obj);
//}
//
//
//static std::unique_ptr<Dummy> PythonMakeDummy(PyObject *func)
//{
//    PyObject *dummy = PyObject_CallFunction(func, NULL);
//    auto dummy_handle = py::handle(dummy);
//    auto cpp_dummy = dummy_handle.cast<std::unique_ptr<Dummy>>();
//    return cpp_dummy;
//}
//
//static std::function<std::unique_ptr<Dummy>()> MakeDummyFactory(PyObject *py_func)
//{
//    return [py_func]() { return PythonMakeDummy(py_func); };
//}
//

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

PYBIND11_MODULE(playground, m)
{
    m.doc() = "Playground to experiment with C++ / Python interworking";

    m.def("MakeDummy", &MakeDummy);

    // Constructor with unique pointer function
    py::class_<Dummy>(m, "Dummy")
            .def(py::init<>())
            .def_property_readonly("id", &Dummy::id);

    py::class_<ConstructVariations>(m, "ConstructVariations")
            .def(py::init([]() { return new ConstructVariations(MakeDummy); }))
//            .def(py::init([](PyObject *func) { return MakeDummyFactory(func); }))
            .def("CallUP_Dummy_func", &ConstructVariations::CallUP_Dummy_func);
}
