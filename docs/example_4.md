---
layout: default
title: Example 4
---

# Example 4 - Traffic light

In the previous examples we saw a pure Python solution
for using a C-library ([example 1](./example_1.md)),
a manually created extension module ([example 2](./example_2.md)),
a SWIG generated extension module ([example 3-1](./example_3-1.md)),
and a `pybind11` generated extenstion module ([example 3-2](./example_3-2.md)).
Each methods required a lot of boilerplate code,
except the `pybind11` method.
Since I want to discuss what you can do when you have C++ and Python code
work with each other, I don't want to get distracted by
boilerplate code that is necessary to make everything work.
For that reason I will use `pybind11` as tool to create
the required wrapper files.


# A Traffic library

In this example we see how we can make proper C++ classes
available in Python.
We do this via a `Traffic` library that contains the following classes:

* A `Light` class, representing a light;
* A `TrafficLight` class with 3 `Light` objects (`red`, `amber`, and `green`).

## Class `Light`

The `Light` object can be in one of the following states:

* `Off`,
* `On`, or 
* `Flashing`.

It has the following pubic methods:

* `GetState()`, to get the state of the light.
* `SetState()`, to set the state of the light.

The header file `light.h` looks like:

```c
#include <iostream>
#include <memory>

class Light
{
public:
    enum class State {Off, On, Flashing};
    static constexpr auto Off = State::Off;
    static constexpr auto On = State::On;
    static constexpr auto Flashing = State::Flashing;

    explicit Light(State state=Off);
    ~Light();
    virtual State GetState() const;
    virtual void SetState(State state);
    static std::unique_ptr<Light> MakeLight();

private:
    ...
};

std::ostream& operator<<(std::ostream& out, Light::State state);
```

The overloading of `operator<<` enables printing the name of the
enum values in stead of their numerical value.

## Class `TrafficLight`

A `TrafficLight` can be in one of the following states:

* `Off`: all lights are off.
* `Warning`: the amber light is flashing.
* `Open`: the green light is on.
* `Closed`: the red light is on.
* `Opening`: transitioning from `Closed` to `Open`.
* `Closing`: transitioning from `Open` to `Closed`.

`TrafficLight` exposes the `MoveTo(state)` method,
to transition to the `state`.
Valid target states for this method are:
 
  * `Open`, to allow traffic to proceed;
  * `Close`, to prohibit traffic to proceed;
  * `Warning`, to set the amber lights to flashing;
  * `Off`, to turn the traffic light off.
 
Other states will be silently ignored.
While the `TrafficLight` transitions to the indicated state,
it displays any required intermediate light patterns as needed
for a certain period.
It will also enforce a minimum time in the final state,
and will not accept any further state-changing requests while
a transition is in progress.

In addition, there are some utility methods:

* `GetLightNames()` returns the names of the lights.
* `GetLightPattern()` returns the states of the lights.
* `GetState()` returns the state of the traffic light.
* `AddCallback(func)` registers a callback function `func` that will be
   called whenever the light pattern changes.

These utility methods are not blocked when a transition is in progress.

The header file `traffic_light.h` looks like:

```c
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "light.h"


class TrafficLight
{
public:
    using LightPattern = std::vector<Light::State>;
    using CallbackFunction = std::function<void(TrafficLight*)>;
    using LightFactory = std::function<std::shared_ptr<Light>()>;
    static const std::vector<std::string> light_names;

    enum class State {Off, Closing, Closed, Opening, Open, Warning};

    explicit TrafficLight(State initial_state = State::Off);
    virtual ~TrafficLight();
    virtual State GetState();
    virtual std::vector<std::string> GetLightNames();
    virtual LightPattern GetLightPattern();
    virtual void MoveTo(State target_state);
    virtual void AddCallback(const CallbackFunction& func);

protected:
    virtual void PrepareTransition(State from_state, State target_state);

private:
    ...
};

std::ostream& operator<<(std::ostream& out, TrafficLight::State state);
```

This class is simple enough to understand,
but has enough complexity to show some of the trickier
parts of C++ and Python interworking.
One of the not-so-trivial aspects is that the class
uses a separate thread to execute state transitions,
and in this way allows the class to remain repsonsive
to calls of any of the utility methods.

## Testing this in C++

When the example is built, the build directory contains an
executable `demo` that exercises these classes.
When you run this, you will see the following output
appear on your terminal, with pauses of a few seconds
between the state changes of the traffic light.

```
$ demo
Testing Light class
Light initialized with Off
Light changed to On
-----------
Testing traffic light
State: Closing (red: Off, amber: On, green: Off)
State: Closed (red: On, amber: Off, green: Off)
State: Open (red: Off, amber: Off, green: On)
State: Closing (red: Off, amber: On, green: Off)
State: Closed (red: On, amber: Off, green: Off)
State: Warning (red: Off, amber: Flashing, green: Off)
State: Off (red: Off, amber: Off, green: Off)

$ 
```

# Creating the extension module

Having a working traffic library,
we now continue with the creation of the wrapper class.
As in [example 3-2](./example_3-2.md),
I use `pybind11` to create the wrapper class.
I will do this step by step.

## The `Light` class

I start with only the `Light` class in the wrapper module.

### First attempt

The first attempt for the wrapper code looks as follows:

```c
#include "pybind11/pybind11.h"
#include "pybind11/functional.h"

#include "light.h"
#include "traffic_light.h"

namespace py = pybind11;

using namespace py::literals;

PYBIND11_MODULE(traffic, m)
{
    m.doc() = "traffic light extension module";
    py::class_<Light>(m, "Light")
            .def(py::init<Light::State>(), "state"_a)
            .def("SetState", &Light::MoveTo, "state"_a, "Set the light state")
            .def("GetState", &Light::GetState, "Get the light state");
}
```

When we build this and run Python in the build directory,
we see that we can import and inspect this module:

```
>>> import traffic
>>> dir(traffic)
['Light', '__doc__', '__file__', '__loader__', '__name__', '__package__', '__spec__']
>>>> help(traffic.Light)
Help on class Light in module traffic:

class Light(pybind11_builtins.pybind11_object)
 |  Method resolution order:
 |      Light
 |      pybind11_builtins.pybind11_object
 |      builtins.object
 |
 |  Methods defined here:
 |
 |  GetState(...)
 |      GetState(self: traffic.Light) -> Light::State
 |
 |      Get the light state
 |
 |  SetState(...)
 |      SetState(self: traffic.Light, state: Light::State) -> None
 |
 |      Set the light state
 |
 |  __init__(...)
 |      __init__(self: traffic.Light, state: Light::State) -> None
 |
 |  ----------------------------------------------------------------------
 |  Static methods inherited from pybind11_builtins.pybind11_object:
 |
 |  __new__(*args, **kwargs) from pybind11_builtins.pybind11_type
 |      Create and return a new object.  See help(type) for accurate signature.

>>>
```

But trying to instantiate a Light object fails,
both with and without an argument.

```
>>> l = traffic.Light()
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
TypeError: __init__(): incompatible constructor arguments. The following argument types are supported:
    1. traffic.Light(state: Light::State)

Invoked with:
>>> l = traffic.Light(0)
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
TypeError: __init__(): incompatible constructor arguments. The following argument types are supported:
    1. traffic.Light(state: Light::State)

Invoked with: 0
>>>

```

So we need to add at least two additional things:

* Indicating the default argument for the constructor, and
* Making the `Light::State` enum class available in Python.

### Default arguments

`pybind11` cannot automatically handle default arguments.
The reason for that is that `pybind11` does not analyze the header files,
but instead looks at the function's (compiled) type information.
And default arguments are not part of that type information.
Default arguments have therefore to be specified explicitly in the
wrapper class.

<pre><code>
    py::class_&lt;Light>(m, "Light")
            .def(py::init&lt;Light::State>(), "state"_a=<mark>Light::Off</mark>)
            .def("MoveTo", &Light::MoveTo, "state"_a, "Set the light state")
            .def("GetState", &Light::GetState, "Get the light state");
</code></pre>

But after building this and trying to import the module in Python,
we get:

```
>>> import traffic
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
ImportError: arg(): could not convert default argument 'state: Light::State' in method '<class 'traffic.Light'>.__init__' into a Python object (type not registered yet?)
>>>
```

So we need to tackle the conversion of `Light::State` into a Python class first
before we can use the `Light` class.

### Converting the nested enum class

To be able to use the nested `enum` class in `Light`,
the wrapper code must be slightly re-organized.

```c
#include "pybind11/pybind11.h"
#include "pybind11/functional.h"

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
            .value("Flashing", Light::Flashing);

    Light.def(py::init<Light::State>(), "state"_a=Light::Off)
            .def("SetState", &Light::MoveTo, "state"_a, "Set the light state")
            .def("GetState", &Light::GetState, "Get the light state");

}
```

The `py::enum` line needs to come before the method definitions,
otherwise the type will not be known by Python.
With these changes we can create and manipulate
`Light` objects from Python.

```text
>>> from traffic import Light
>>> l = Light()
>>> l.GetState()
State.Off
>>> l.MoveTo(Light.State.Flashing)
>>> l.GetState()
State.Flashing
>>>
```

We have now surfaced two minor annoyances, however.

 * Using getters and setters is not very Pythonic.
   The Python way to do this is to use properties,
   which can have user-defined read and write semantics.
 * Having to spell the light states as `Light.State.Flashing`
   is too much typing.
   It would be nice if the values could be brought directly under the
   `Light` class, just as in the C++ code.

### Replace getter and setter with property

To remove the getter and setter function, and to use
a Python property instead,
`pybind11` offers the `.def_property` method for the module definition.

There is also a `.def_property_readonly` method to define read-only properties.
And by providing a `nullptr` for the getter method, `.def_property` can also be
used to define write-only properties.

```c
#include "pybind11/pybind11.h"
#include "pybind11/functional.h"

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
            .value("Flashing", Light::Flashing);

    Light.def(py::init<Light::State>(), "state"_a = Light::Off)
            .def_property("state", &Light::GetState, &Light::MoveTo, "The light state");
}
```

With these changes we can now read and write the state in Python as if it is
an attribute of the `Light` object.

```text
>>> from traffic import Light
>>> l = Light()
>>> l.state
State.Off
>>> l.state = Light.State.Flashing
>>> l.state
State.Flashing
>>>
```

### Make enum values available directly in `Light` class

Making the enum values available in its surrounding scope
is easily done by calling `.export_values()` on the `py::enum` definition:

```c
    py::enum_<Light::State>(Light, "State")
            .value("Off", Light::Off)
            .value("On", Light::On)
            .value("Flashing", Light::Flashing)
            .export_values();
```

This makes the enum values directly available inside the Python `Light` class,
with even less effort than was needed for the C++ case.

```text
>>> from traffic import Light
>>> dir(Light)
['Flashing', 'Off', 'On', 'State', '__class__', ..., 'state']
>>> Light.Off
State.Off
>>> l = Light()
>>> l.state
State.Off
>>> l.state = Light.On
>>> l.state
State.On
>>>
```

## The `TrafficLight` class

Now that the `Light` class is properly wrapped,
we continue with the `TrafficLight` class.

### Basic functionality

We start with getting the basic functionality.
In this case that means: creating a TrafficLight instance,
querying the state, light pattern, and light names,
and performing a transition.
The following code is added to `traffic.cpp`

```c
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
            .def_property_readonly("names", &TrafficLight::GetLightNames, "The names of the lights");
```

When we compile this, we can create a traffic light and perform a transition.

```text
>>> import traffic
>>> t = traffic.TrafficLight()
>>> t.state
State.Off
>>> t.MoveTo(t.State.Closed)
>>> t.state
State.Closed
```

We can also see the intermediate states, proving that the threading
mechanism inside the class works.
To that end I define a small function that queries the state
of the traffic light every half second, until the target
state is reached.

```text
>>> import time
>>> def waitfor(tl, state):
...     while (s := tl.state) != state:
...         print(s)
...         time.sleep(0.5)
...     print(s)
...
>>> t.MoveTo(t.State.Open); waitfor(t, t.State.Open)
State.Opening
State.Open
>>> t.MoveTo(t.State.Closed); waitfor(t, t.State.Closed)
State.Closing
State.Closing
State.Closing
State.Closing
State.Closed
>>>
```

Querying the light pattern or the names of the lights fails though:

```text
>>> t.pattern
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
TypeError: Unable to convert function return value to a Python type! The signature was
        (arg0: traffic.TrafficLight) -> std::vector<Light::State,std::allocator<Light::State> >

Did you forget to `#include <pybind11/stl.h>`? Or <pybind11/complex.h>,
<pybind11/functional.h>, <pybind11/chrono.h>, etc. Some automatic
conversions are optional and require extra headers to be included
when compiling your pybind11 module.
>>>
```

Both of these return a `std::vector` container, which is not automatically
converted.
Fortunately the error message is again very helpful, and including `pybind11/stl.h`
solves this problem,
as we can see after a rebuild:

```text
>>> import traffic
>>> t = traffic.TrafficLight()
>>> t.state
State.Off
>>> t.names
['red', 'amber', 'green']
>>> t.pattern
[State.Off, State.Off, State.Off]
>>>
```

We can also verify that `state`, `names`, and `patterns` are indeed read-only,
by trying to assign a value to them:

```text
>>> t.state = t.State.Warning
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
AttributeError: can't set attribute
>>>
```
### Adding a callback function

So far we have skipped one TrafficLight method that we really
should have: the `AddCallback` method.
Let's do that now.
We add a line for the `AddCallback` method to the wrapper definition
for `TrafficLight`:

```
    TrafficLight.def(py::init<TrafficLight::State>(),
                     "initial_state"_a = TrafficLight::State::Off)
            .def("MoveTo", &TrafficLight::MoveTo, "target_state"_a)
            .def_property_readonly("state", &TrafficLight::GetState, "The state of the traffic light")
            .def_property_readonly("pattern", &TrafficLight::GetLightPattern, "The light pattern of the traffic light")
            .def_property_readonly("names", &TrafficLight::GetLightNames, "The names of the lights")
            .def("AddCallback", &TrafficLight::AddCallback, "Add a callback method");
```

And after building this, we see that it just works:

```text
>>> import traffic
>>> t = traffic.TrafficLight()
>>> def monitor(tl):
...     print(tl.state, tl.pattern)
...
>>> t.AddCallback(monitor)
>>> t.MoveTo(t.State.Closed)
>>> State.Closing [State.Off, State.On, State.Off]
State.Closed [State.On, State.Off, State.Off]
```

Let's see if we can do something wicked, like triggering a transition
to `Closed` as soon as the traffic light reaches the `Open` state.
You probably have encountered this type of traffic light.

```text
>>> def force_closed(tl):
...     if (tl.state == tl.State.Open):
...         tl.MoveTo(tl.State.Closed)
...
>>> t.AddCallback(force_closed)
>>> t.state
State.Closed
>>> t.MoveTo(t.State.Open)
>>> State.Open (red: Off, amber: Off, green: On)
State.Closing (red: Off, amber: On, green: Off)
State.Closed (red: On, amber: Off, green: Off)
```

