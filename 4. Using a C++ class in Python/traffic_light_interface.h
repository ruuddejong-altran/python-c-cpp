#ifndef PYTHON_C_C_EXAMPLE_4_TRAFFIC_LIGHT_INTERFACE_H
#define PYTHON_C_C_EXAMPLE_4_TRAFFIC_LIGHT_INTERFACE_H

#include "light_interface.h"

class ITrafficLight
{
public:
    virtual  ~ITrafficLight() = default;

    using LS = Light::State;
    using LightPattern = std::vector<LS>;
    using CallbackFunction = std::function<void(TrafficLight*)>;

    enum class State {Off, Closing, Closed, Opening, Open, Warning};

    explicit TrafficLight(const std::function<std::shared_ptr<Light>()>& light_factory = &Light::MakeLight,
                          State initial_state = State::Off);
    virtual ~TrafficLight();
    virtual State GetState();
    virtual std::vector<std::string> GetLightNames();
    virtual LightPattern GetLightPattern();
    virtual void Open();
    virtual void Close();
    virtual void Warning();
    virtual void Off();
    virtual void AddCallback(const CallbackFunction& func);

};


#endif //PYTHON_C_C_EXAMPLE_4_TRAFFIC_LIGHT_INTERFACE_H
