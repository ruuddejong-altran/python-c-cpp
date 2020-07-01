#include "example.h"

#include <utility>

Base::Base(std::string label) : label_(std::move(label))
{
}

Base::~Base() = default;

std::string Base::GetLabel()
{
    return label_;
}

void Base::SetLabel(std::string label)
{
    label_ = std::move(label);
}

std::string Base::Repr()
{
    return std::string("<Base(\"") + GetLabel() + "\")>";
}

std::string DerivedCPP::Repr()
{
    return std::string("<DerivedCPP(\"") + GetLabel() + "\")>";
}

DerivedCPP::~DerivedCPP() = default;

void ObjectRepresentation(const std::shared_ptr<Base>& object)
{
    std::cout << object->Repr() << std::endl;
}