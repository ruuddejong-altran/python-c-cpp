---
layout: default
title: Polymorphism
---

# Polymorphism

For a proper interworking between Python and C++,
we must be able to instantiate Python objects
from C++ classes,
and use these as if they were C++ objects.
And the same goes for Python-defined subclasses
of C++ virtual classes.
These should also be handled the same as C++ derived classes.

## C++ base and derived class

To investigate this, I have defined a C++ base and derived class,
and a utility function:

```c
class Base
{
public:
    explicit Base(std::string label);
    virtual ~Base();
    std::string GetLabel();
    void SetLabel(std::string label);
    virtual std::string Repr();

private:
    std::string label_;
};

class DerivedCPP : public Base
{
public:
    using Base::Base;
    ~DerivedCPP() override;
    std::string Repr() override;
};

void ObjectRepresentation(const std::shared_ptr<Base>& object);
```

The relevant implementation is (I have omitted the trivial constructor and the
setting and getting of the label):

```c
std::string Base::Repr()
{
    return std::string("<Base(\"") + GetLabel() + "\")>";
}

std::string DerivedCPP::Repr()
{
    return std::string("<DerivedCPP(\"") + GetLabel() + "\")>";
}

void ObjectRepresentation(const std::shared_ptr<Base>& object)
{
    std::cout << object->Repr() << std::endl;
}
```

A simple C++ test program to verify that this works shows the expected results:

```c
int main()
{
    std::shared_ptr<Base> base_object = std::make_shared<Base>("object 1");
    std::shared_ptr<Base> derived_object = std::make_shared<DerivedCPP>("object 2");
    std::cout << "base object: ";
    ObjectRepresentation(base_object);
    std::cout << "derived object: ";
    ObjectRepresentation(derived_object);
    std::cout << "Changing label on derived object" << std::endl;
    derived_object->SetLabel("new label");
    ObjectRepresentation(derived_object);

    return 0;
}
```

The output is:
```text
base object: <Base("object 1")>
derived object: <DerivedCPP("object 2")>
Changing label on derived object
<DerivedCPP("new label")>

Process finished with exit code 0
```

## Python binding

To make this available for Python, I have the following binding code:

```c
#include "pybind11/pybind11.h"

#include "example.h"

namespace py = pybind11;

using namespace py::literals;

PYBIND11_MODULE(polymorphism, m)
{
    m.doc() = "Polymorphism examples";
    py::class_<Base>(m, "Base")
            .def(py::init<std::string>())
            .def_property("label", &Base::GetLabel, &Base::SetLabel)
            .def("Repr", &Base::Repr);
    m.def("ObjectRepresentation", &ObjectRepresentation);
}
```

When we try to use this in Python,
we see that we can create a `Base` object,
but we cannot use it as argument to the `ObjectRepresentation` function:

```text
>>> from polymorphism import Base, ObjectRepresentation
>>> b = Base("Python-1")
>>> print(b)
<Base("Python-1")>
>>> ObjectRepresentation(b)
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
RuntimeError: Unable to load a custom holder type from a default-holder instance
>>>
```

The error message is somewhat cryptic.
But the *holder type* is basically the type that is used to wrap the C++ object
in order to make it a Python object.
In particular, the holding type takes care of the management
of the (Python) references.
See the section on [type conversions](./conversions.md)
for more information about wrapping and type conversion.

By default, `pybind11` uses a `std::unique_ptr` as holding type,
because that makes memory management easy:
the object is destroyed when the object's reference count
in Python goes to zero
(more on that in the section about [ownership](./ownership.md)).

But the function `ObjectRepresentation` expects a `std::shared_ptr`,
and there is no defined mapping of a `std::unique_ptr` to a `std::shared_ptr`.
Hence the error.

In this case we can solve this by specifying in the binding code's
template argument
that the holding type should be a `std::shared_ptr`:

```c
    ...
    py::class_<Base, std::shared_ptr<Base>>(m, "Base")
    ...
```

With this change, the Python object can be used as argument to `ObjectRepresentation`:

```text
>>> from polymorphism import Base, ObjectRepresentation
>>> b = Base("Python-1")
>>> print(b.Repr())
<Base("Python-1")>
>>> ObjectRepresentation(b)
<Base("Python-1")>
>>>
```

But when we try to use a (Python) subclass,
the result is not quite what we would like to have:

```text
>>> class PythonDerived(Base):
...     def Repr(self):
...         return f'<PythonDerived("{self.label}")>'
...
>>> d = PythonDerived('Derived-1')
>>> print(d.Repr())
<PythonDerived("Derived-1")>
<>>> ObjectRepresentation(d)
<Base("Derived-1")>
>>>
```

We see that the `ObjectRepresentation` use the method from the `Base` class,
not from the derived class.
We loose polymorphism in this way.

When we follow what happens here, it becomes obvious why
this does not work.
When a Python object is "transferred" to C++,
it is converted to a C++ object.
For Python objects that already wrap a C++ object
(such as a `Base` object, but also a `PythonDerived` object)
this means that
the wrapper around the C++ object is simply removed,
and the resulting C++ object is used.
And for both the `Base` and the `PythonDerived` objects,
that C++ object is a C++ `Base` instance.

What we want to achieve is that the C++ code in `ObjectRepresentation`
uses the Python-defined method `Repr`.
There is no way that we can achieve this directly: the C++ compiler
simply cannot know what Python-defined derived classes will be defined.
We have to add an indirection, a so-called *trampoline class*.

A *trampoline class* is aptly named: you jump from Python into it the C++ code,
and you immediately jump out again back to Python code.
In order to make polymorphism work, the trampoline class must
be a derived class from Base.

```c
class _BaseTrampoline : public Base
{
public:
    using Base::Base;
    std::string Repr() override
    {
        PYBIND11_OVERLOAD(std::string, Base, Repr, );
    }
};
```

The `PYBIND11_OVERLOAD` macro takes care of the code to
call the Python method of a derived class.
There is a related `PYBIND11_OVERLOAD_PURE` macro that should
be used for pure virtual functions,
and corresponding `PYBIND11_OVERLOAD[_PURE]_NAME` macros that must
be used when the Python method's name is different from the
C++ name.

The final step is to make the binding code aware of the the
trampoline helper class.
This is (again) done by specifying the trampoline class
as an extra template argument in the `py::class_<..>` call:

```c
    py::class_<Base, _BaseTrampoline, std::shared_ptr<Base>>(m, "Base")
            .def(py::init<std::string>())
            .def_property("label", &Base::GetLabel, &Base::SetLabel)
            .def("Repr", &Base::Repr);
```

Now the behavior is as expected:

```text
>>> from polymorphism import Base, ObjectRepresentation
>>> class PythonDerived(Base):
...     def Repr(self):
...         return f'<PythonDerived("{self.label}")>'
...
>>> d = PythonDerived("derived")
>>> print(d.Repr())
<PythonDerived("derived")>
>>> ObjectRepresentation(d)
<PythonDerived("derived")>
>>>
```

## Private and protected virtual methods

When the virtual method you want to override
is a *protected* method,
then you need an intermediate helper class
that derives from the base class.
This helper class changes the access modifier of the virtual method
to *public*.
The binding code can then reference the method from the
helper class in stead of the original class.

But if the virtual method is *private* then this is not possible.
Private virtual methods are typically found when
the *template pattern* is used in the class definitions.
With the template pattern, the interface is defined
with non-virtual *public* methods,
and the implementation is defined with virtual *private* methods.
Since the virtual methods are private,
the binding code cannot reference them,
and hence there can be no trampoline method for then.
In this case polymorphism for
Python-defined subclasses is simply impossible,
or at least requires coding tricks that are way beyond
the scope of this text.
