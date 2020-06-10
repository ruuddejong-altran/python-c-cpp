#ifndef PYTHON_C_C_EXAMPLE_4_LIGHT_H
#define PYTHON_C_C_EXAMPLE_4_LIGHT_H

#include <iostream>


class Light
{
public:
    enum class State {Off, On, Flashing};

    Light(State state=State::Off);
    virtual ~Light() = default;
    virtual State GetState() const;
    virtual void SetState(State state);

private:
    State state_;
};

std::ostream& operator<<(std::ostream& out, const Light::State state);

#endif //PYTHON_C_C_EXAMPLE_4_LIGHT_H
