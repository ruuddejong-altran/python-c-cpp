#include "pybind11/pybind11.h"

#include "example.h"

namespace py = pybind11;

using namespace py::literals;

class _BaseTrampoline : public Base
{
public:
    using Base::Base;
    std::string Repr() override
    {
        PYBIND11_OVERLOAD(std::string, Base, Repr, );
    }
};

PYBIND11_MODULE(polymorphism, m)
{
    m.doc() = "Polymorphism examples";
    py::class_<Base, _BaseTrampoline, std::shared_ptr<Base>>(m, "Base")
            .def(py::init<std::string>())
            .def_property("label", &Base::GetLabel, &Base::SetLabel)
            .def("Repr", &Base::Repr);
    m.def("ObjectRepresentation", &ObjectRepresentation);
}
