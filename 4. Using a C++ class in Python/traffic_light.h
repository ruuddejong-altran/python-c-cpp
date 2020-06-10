#ifndef PYTHON_C_C_EXAMPLE_4_TRAFFIC_LIGHT_H
#define PYTHON_C_C_EXAMPLE_4_TRAFFIC_LIGHT_H

#include <array>
#include <condition_variable>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>
#include "light.h"


class TrafficLight
{
public:
    static const size_t number_of_lights = 3;

    using LightNames = std::array<std::string_view, number_of_lights>;
    static constexpr const LightNames light_names = {"red", "amber", "green"};

    using LS = Light::State;
    using LightPattern = std::array<LS, number_of_lights>;
    enum class State {Off, Closing, Closed, Opening, Open, Warning};

    explicit TrafficLight(const std::function<std::shared_ptr<Light>()>& light_factory,
                          State initial_state = State::Off);
    virtual ~TrafficLight();
    virtual State GetState();
    virtual LightNames GetLightNames();
    virtual LightPattern GetLightPattern();
    virtual void Open();
    virtual void Close();
    virtual void Warning();
    virtual void Off();
    using CallbackFunction = std::function<void(TrafficLight*)>;
    virtual void AddCallback(const CallbackFunction& func);

protected:
    void Init(State initial_state);
    virtual void SetState(State target_state);
    virtual void SetLightPattern(LightPattern pattern);
    virtual void PrepareTransition(State from_state, State target_state);

private:
    using TransitionElement = std::tuple<LightPattern, int>;
    using TransitionSequence = std::vector<TransitionElement>;

    void RunCallbackFunction(const CallbackFunction& func);
    void RunTransition(State to_state);
    void SetLightPatternAndWait(LightPattern pattern, int delay_ms);

    State current_state_;
    std::vector<std::shared_ptr<Light>> lights_;
    std::map<std::string_view, std::shared_ptr<Light>> light_map_;
    std::vector<CallbackFunction> state_change_cb_list_;
    TransitionSequence transition_sequence_;
    std::thread transition_thread_;
    std::mutex lights_mutex_;
    std::mutex transition_mutex_;
};

std::ostream& operator<<(std::ostream& out, TrafficLight::State state);

#endif //PYTHON_C_C_EXAMPLE_4_TRAFFIC_LIGHT_H
