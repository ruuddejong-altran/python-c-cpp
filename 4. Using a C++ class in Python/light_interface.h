//
// Created by Ruud on 11-6-2020.
//

#ifndef PYTHON_C_C_EXAMPLE_4_LIGHT_INTERFACE_H
#define PYTHON_C_C_EXAMPLE_4_LIGHT_INTERFACE_H

#include <memory>


class ILight
{
public:
    virtual ~ILight() = default;
    virtual LightState GetState() const = 0;
    virtual void SetState(LightState state) = 0;
};


#endif //PYTHON_C_C_EXAMPLE_4_LIGHT_INTERFACE_H
