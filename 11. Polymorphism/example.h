#ifndef PYTHON_C_C_POLYMORPHISM_EXAMPLE_H
#define PYTHON_C_C_POLYMORPHISM_EXAMPLE_H

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>

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

#endif //PYTHON_C_C_POLYMORPHISM_EXAMPLE_H
