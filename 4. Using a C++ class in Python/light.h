#ifndef PYTHON_C_C_EXAMPLE_4_LIGHT_H
#define PYTHON_C_C_EXAMPLE_4_LIGHT_H

#include <iostream>

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
    State state_;
};

std::ostream& operator<<(std::ostream& out, Light::State state);

#endif //PYTHON_C_C_EXAMPLE_4_LIGHT_H
