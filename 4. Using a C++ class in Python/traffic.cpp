#include "pybind11/pybind11.h"
#include "pybind11/functional.h"
#include "pybind11/stl.h"

#include "light.h"
#include "traffic_light.h"

namespace py = pybind11;

using namespace py::literals;


PYBIND11_MODULE(traffic, m)
{
    m.doc() = "traffic light extension module";

    py::class_<Light> Light(m, "Light");

    py::enum_<Light::State>(Light, "State")
            .value("Off", Light::Off)
            .value("On", Light::On)
            .value("Flashing", Light::Flashing)
            .export_values();

    Light.def(py::init<Light::State>(), "state"_a = Light::Off)
            .def_property("state", &Light::GetState, &Light::SetState, "The light state");

    py::class_<TrafficLight> TrafficLight(m, "TrafficLight");

    py::enum_<TrafficLight::State>(TrafficLight, "State")
            .value("Off", TrafficLight::State::Off)
            .value("Open", TrafficLight::State::Open)
            .value("Opening", TrafficLight::State::Opening)
            .value("Closed", TrafficLight::State::Closed)
            .value("Closing", TrafficLight::State::Closing)
            .value("Warning", TrafficLight::State::Warning);

    TrafficLight.def(py::init<TrafficLight::State>(),
                     "initial_state"_a = TrafficLight::State::Off)
            .def("MoveTo", &TrafficLight::MoveTo, "target_state"_a)
            .def_property_readonly("state", &TrafficLight::GetState, "The state of the traffic light")
            .def_property_readonly("pattern", &TrafficLight::GetLightPattern, "The light pattern of the traffic light")
            .def_property_readonly("names", &TrafficLight::GetLightNames, "The names of the lights")
            .def("AddCallback", &TrafficLight::AddCallback, "Add a callback method");
}
