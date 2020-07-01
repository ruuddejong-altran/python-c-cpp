#include <iostream>

#include "example.h"

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
